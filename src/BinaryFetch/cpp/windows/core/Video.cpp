// Video.cpp
// Windows-only. Windows Media Foundation source reader -> scale -> PixelEncoders.

#include "Video.h"
#include "PixelEncoders.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wrl/client.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;

namespace {



// cell size: ask the terminal (CSI 16 t), fall back to Win32
bool queryCellSizeFromTerminalVideo(int& outW, int& outH) {
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) return false;
    if (hIn == nullptr || hOut == nullptr) return false;

    DWORD oldMode = 0;
    if (!GetConsoleMode(hIn, &oldMode)) return false;
    SetConsoleMode(hIn, ENABLE_VIRTUAL_TERMINAL_INPUT);
    FlushConsoleInputBuffer(hIn);

    const char* query = "\033[16t";
    DWORD written = 0;
    BOOL wrote = WriteFile(hOut, query, (DWORD)strlen(query), &written, nullptr);

    std::string response;
    if (wrote) {
        char buf[64];
        for (int tries = 0; tries < 10; ++tries) {
            if (WaitForSingleObject(hIn, 20) != WAIT_OBJECT_0) {
                if (!response.empty()) break;
                continue;
            }
            DWORD read = 0;
            if (!ReadFile(hIn, buf, sizeof(buf) - 1, &read, nullptr) || read == 0) break;
            buf[read] = '\0';
            response += buf;
            if (response.find('t') != std::string::npos) break;
        }
    }
    SetConsoleMode(hIn, oldMode);

    int h = 0, w = 0;
    size_t pos = response.find("[6;");
    if (pos != std::string::npos &&
        sscanf(response.c_str() + pos, "[6;%d;%dt", &h, &w) == 2 && h > 0 && w > 0) {
        outW = w; outH = h;
        return true;
    }
    return false;
}

COORD getTerminalCellSizeVideo() {
    int qw = 0, qh = 0;
    if (queryCellSizeFromTerminalVideo(qw, qh) &&
        qw >= 4 && qw <= 64 && qh >= 8 && qh <= 128) {
        return { (SHORT)qw, (SHORT)qh };
    }
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fi{};
    fi.cbSize = sizeof(fi);
    if (GetCurrentConsoleFontEx(hOut, FALSE, &fi)) {
        int w = fi.dwFontSize.X, h = fi.dwFontSize.Y;
        if (w >= 4 && w <= 64 && h >= 8 && h <= 128) return { (SHORT)w, (SHORT)h };
    }
    return { 8, 16 };
}



} // anonymous namespace

// Media Foundation state (private to this file)

struct TerminalVideo::MFState {
    ComPtr<IMFSourceReader> reader;
    bool mfStarted = false;
    bool comInited = false;
    int srcW = 0, srcH = 0;
    LONG stride = 0;              // bytes per row; negative = bottom-up in memory
    double nativeFps = 0.0;
    LONGLONG targetTime = 0;      // 100 ns units
    bool needAnchor = true;
    HRESULT lastHr = S_OK;
    std::vector<int> xmap;        // dest x -> source x

    // Visible picture area inside the decoded frame (see computeCrop).
    int cropX = 0, cropY = 0, cropW = 0, cropH = 0;

    // Fade-in from black on launch. fadeInMs = 0 turns it off.
    // framesShown is reset in load() but NOT in restart(), so looping the
    // video never fades again.
    int fadeInMs = 800;
    int fadeFrames = 0;
    int framesShown = 0;

    // Decoders pad frames to block sizes; the padding can hold garbage that
    // shows up as a green line. Use the display aperture if the decoder gives
    // one, then also stay EDGE_TRIM pixels away from the right/bottom edge.
    // Set EDGE_TRIM to 0 to disable the extra margin.
    void computeCrop(IMFMediaType* t) {
        cropX = 0; cropY = 0; cropW = srcW; cropH = srcH;
        MFVideoArea area{};
        if (SUCCEEDED(t->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE,
                                 reinterpret_cast<UINT8*>(&area), sizeof(area), nullptr))) {
            int x = (int)area.OffsetX.value, y = (int)area.OffsetY.value;
            int w = (int)area.Area.cx,       h = (int)area.Area.cy;
            if (w > 0 && h > 0 && x >= 0 && y >= 0 && x + w <= srcW && y + h <= srcH) {
                cropX = x; cropY = y; cropW = w; cropH = h;
            }
        }
        constexpr int EDGE_TRIM = 2;
        if (cropW > EDGE_TRIM * 4) cropW -= EDGE_TRIM;
        if (cropH > EDGE_TRIM * 4) cropH -= EDGE_TRIM;
    }

    // Re-reads frame size and stride from the reader's current output type.
    bool refresh() {
        ComPtr<IMFMediaType> t;
        if (FAILED(reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &t))) return false;
        UINT32 w = 0, h = 0;
        if (FAILED(MFGetAttributeSize(t.Get(), MF_MT_FRAME_SIZE, &w, &h)) || w == 0 || h == 0) return false;
        UINT32 st = 0;
        LONG s = SUCCEEDED(t->GetUINT32(MF_MT_DEFAULT_STRIDE, &st)) ? (LONG)st : (LONG)(w * 4);
        if (s == 0) s = (LONG)(w * 4);
        srcW = (int)w; srcH = (int)h; stride = s;
        computeCrop(t.Get());
        return true;
    }
};

// TerminalVideo

TerminalVideo::TerminalVideo()
    : rowSpan(0), colSpan(0), paddingUp(0), paddingLeft(0), paddingRight(0),
      cellHeightPx(0), cellWidthPx(0), fps(15), outW(0), outH(0),
      loaded(false), flipVertical(false), mf(new MFState()) {}

TerminalVideo::~TerminalVideo() {
    close();
    if (mf->mfStarted) MFShutdown();
    if (mf->comInited) CoUninitialize();
    delete mf;
}

void TerminalVideo::setPadding(int up, int left, int right) {
    paddingUp = up; paddingLeft = left; paddingRight = right;
}
int  TerminalVideo::getPaddingUp() const    { return paddingUp; }
int  TerminalVideo::getPaddingLeft() const  { return paddingLeft; }
int  TerminalVideo::getPaddingRight() const { return paddingRight; }
int  TerminalVideo::getRowSpan() const      { return rowSpan; }
int  TerminalVideo::getColSpan() const      { return colSpan; }
bool TerminalVideo::isLoaded() const        { return loaded; }
void TerminalVideo::setCellHeightPx(int px) { cellHeightPx = px; }
void TerminalVideo::setCellWidthPx(int px)  { cellWidthPx = px; }
void TerminalVideo::setFps(int f)           { fps = std::clamp(f, 1, 60); }
int  TerminalVideo::getFps() const          { return fps; }
void TerminalVideo::setFlipVertical(bool f) { flipVertical = f; }
void TerminalVideo::setFadeInMs(int ms)     { mf->fadeInMs = std::clamp(ms, 0, 5000); }

void TerminalVideo::close() {
    mf->reader.Reset();
}

bool TerminalVideo::openReader() {
    mf->reader.Reset();

    int n = MultiByteToWideChar(CP_UTF8, 0, videoPath.c_str(), -1, nullptr, 0);
    if (n <= 0) { mf->lastHr = E_INVALIDARG; return false; }
    std::wstring wpath((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, videoPath.c_str(), -1, &wpath[0], n);

    HRESULT hr;
    ComPtr<IMFAttributes> attrs;
    hr = MFCreateAttributes(&attrs, 1);
    if (FAILED(hr)) { mf->lastHr = hr; return false; }
    attrs->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

    ComPtr<IMFSourceReader> reader;
    hr = MFCreateSourceReaderFromURL(wpath.c_str(), attrs.Get(), &reader);
    if (FAILED(hr)) { mf->lastHr = hr; return false; }

    // Video only: audio streams are never selected, so never decoded.
    reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    hr = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    if (FAILED(hr)) { mf->lastHr = hr; return false; }

    ComPtr<IMFMediaType> want;
    hr = MFCreateMediaType(&want);
    if (FAILED(hr)) { mf->lastHr = hr; return false; }
    want->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    want->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, want.Get());
    if (FAILED(hr)) { mf->lastHr = hr; return false; }

    ComPtr<IMFMediaType> got;
    hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &got);
    if (FAILED(hr)) { mf->lastHr = hr; return false; }

    UINT32 w = 0, h = 0;
    hr = MFGetAttributeSize(got.Get(), MF_MT_FRAME_SIZE, &w, &h);
    if (FAILED(hr) || w == 0 || h == 0) { mf->lastHr = FAILED(hr) ? hr : E_FAIL; return false; }

    UINT32 st = 0;
    LONG stride = SUCCEEDED(got->GetUINT32(MF_MT_DEFAULT_STRIDE, &st)) ? (LONG)st : (LONG)(w * 4);
    if (stride == 0) stride = (LONG)(w * 4);

    UINT32 num = 0, den = 0;
    double nativeFps = 0.0;
    if (SUCCEEDED(MFGetAttributeRatio(got.Get(), MF_MT_FRAME_RATE, &num, &den)) && den != 0)
        nativeFps = (double)num / (double)den;

    mf->reader = reader;
    mf->srcW = (int)w;
    mf->srcH = (int)h;
    mf->stride = stride;
    mf->nativeFps = nativeFps;
    mf->targetTime = 0;
    mf->needAnchor = true;
    mf->computeCrop(got.Get());
    return true;
}

bool TerminalVideo::load(const std::string& path, int sizePercent) {
    loaded = false;
    close();
    videoPath = path;

    if (!mf->mfStarted) {
        HRESULT hrc = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        mf->comInited = SUCCEEDED(hrc);   // RPC_E_CHANGED_MODE: COM already up, fine
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            std::cerr << "Warning: Media Foundation could not start (0x"
                      << std::hex << (unsigned long)hr << std::dec << ").\n";
            return false;
        }
        mf->mfStarted = true;
    }

    if (!openReader()) {
        std::cerr << "Warning: could not open video: " << path << " (HRESULT 0x"
                  << std::hex << (unsigned long)mf->lastHr << std::dec << ")\n"
                  << "  Check the path, and that Windows can play this format "
                     "(MP4/H.264 is the safe choice).\n";
        return false;
    }

    outW = mf->srcW;
    outH = mf->srcH;
    if (sizePercent > 0 && sizePercent != 100) {
        outW = std::max(1, (int)(mf->srcW * (sizePercent / 100.0)));
        outH = std::max(1, (int)(mf->srcH * (sizePercent / 100.0)));
    }
    rgba.assign((size_t)outW * outH * 4, 0);

    mf->xmap.assign((size_t)outW, 0);
    for (int x = 0; x < outW; ++x)
        mf->xmap[(size_t)x] = mf->cropX + (int)((long long)x * mf->cropW / outW);

    // fps can't exceed the video's own frame rate
    if (mf->nativeFps > 0.5)
        fps = std::max(1, std::min(fps, (int)std::floor(mf->nativeFps + 0.5)));

    mf->fadeFrames  = (mf->fadeInMs > 0) ? std::max(1, fps * mf->fadeInMs / 1000) : 0;
    mf->framesShown = 0;

    COORD cell = getTerminalCellSizeVideo();
    int cw = (cellWidthPx > 0) ? cellWidthPx : cell.X;
    if (cw <= 0) cw = 8;
    int ch = (cellHeightPx > 0) ? cellHeightPx : cell.Y;
    if (ch <= 0) ch = 16;
    const bool half = (encodeOpts.protocol == GraphicsProtocol::HalfBlock);
    colSpan = half ? outW : (outW + cw - 1) / cw;
    rowSpan = half ? (outH + 1) / 2 : (outH + ch - 1) / ch;

    loaded = true;
    return true;
}

bool TerminalVideo::nextFrame(std::string& out) {
    if (!mf->reader) return false;

    const LONGLONG interval = 10000000LL / std::max(1, fps);
    LONGLONG target = mf->targetTime;

    for (;;) {
        DWORD flags = 0;
        LONGLONG ts = 0;
        ComPtr<IMFSample> sample;
        HRESULT hr = mf->reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0,
                                            nullptr, &flags, &ts, &sample);
        if (FAILED(hr)) return false;
        if (flags & (MF_SOURCE_READERF_ENDOFSTREAM | MF_SOURCE_READERF_ERROR)) return false;

        // The decoder can change frame size / stride mid-stream. Re-read them,
        // otherwise rows are read with the wrong width and the picture shears.
        if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
            if (!mf->refresh()) return false;
            mf->xmap.assign((size_t)outW, 0);
            for (int x = 0; x < outW; ++x)
                mf->xmap[(size_t)x] = mf->cropX + (int)((long long)x * mf->cropW / outW);
        }
        if (!sample) continue;

        if (mf->needAnchor) {          // first frame after open/seek sets the clock
            target = ts;
            mf->needAnchor = false;
        }
        if (ts + interval / 2 < target) continue;   // running behind: drop this frame

        ComPtr<IMFMediaBuffer> buf;
        if (FAILED(sample->ConvertToContiguousBuffer(&buf))) return false;

        const int srcW = mf->srcW, srcH = mf->srcH;

        // Ask the buffer for its REAL row pitch. 'top' is the first image row
        // (top-down order) and 'pitch' is the signed byte distance between rows.
        const BYTE* top = nullptr;
        LONG pitch = 0;
        bool ok = false;
        bool locked2d = false;

        ComPtr<IMF2DBuffer> buf2d;
        if (SUCCEEDED(buf.As(&buf2d))) {
            BYTE* scan0 = nullptr;
            LONG p = 0;
            if (SUCCEEDED(buf2d->Lock2D(&scan0, &p))) {
                locked2d = true;
                top = scan0;
                pitch = p;
                ok = (scan0 != nullptr && p != 0);
            }
        }

        if (!locked2d) {
            // Fallback: plain buffer, trust the media type's stride.
            BYTE* data = nullptr;
            DWORD maxLen = 0, curLen = 0;
            if (FAILED(buf->Lock(&data, &maxLen, &curLen))) return false;
            LONG rowBytes = std::abs(mf->stride);
            if (rowBytes < srcW * 4) rowBytes = srcW * 4;
            ok = (size_t)curLen >= (size_t)rowBytes * (srcH - 1) + (size_t)srcW * 4;
            if (mf->stride < 0) { top = data + (size_t)(srcH - 1) * rowBytes; pitch = -rowBytes; }
            else                { top = data; pitch = rowBytes; }
        }

        // Fade-in brightness, 0..256 (256 = full). Smoothstep curve so it
        // eases in and out instead of ramping linearly.
        int mul = 256;
        if (mf->fadeFrames > 0 && mf->framesShown < mf->fadeFrames) {
            float t = (float)(mf->framesShown + 1) / (float)mf->fadeFrames;
            mul = (int)(t * t * (3.0f - 2.0f * t) * 256.0f);
        }

        if (ok) {
            // RGB32 in memory is B,G,R,X. top/pitch are already top-down.
            for (int y = 0; y < outH; ++y) {
                int sy = (int)((long long)y * mf->cropH / outH);
                int row = mf->cropY + (flipVertical ? (mf->cropH - 1 - sy) : sy);
                const BYTE* srow = top + (ptrdiff_t)row * pitch;
                unsigned char* d = rgba.data() + (size_t)y * outW * 4;
                for (int x = 0; x < outW; ++x) {
                    const BYTE* s = srow + (size_t)mf->xmap[(size_t)x] * 4;
                    d[0] = (unsigned char)((s[2] * mul) >> 8);
                    d[1] = (unsigned char)((s[1] * mul) >> 8);
                    d[2] = (unsigned char)((s[0] * mul) >> 8);
                    d[3] = 255;
                    d += 4;
                }
            }
        }
        if (locked2d) buf2d->Unlock2D(); else buf->Unlock();
        if (!ok) return false;

        mf->targetTime = target + interval;
        mf->framesShown++;
        return encodePixels(rgba.data(), outW, outH, encodeOpts, out);
    }
}

bool TerminalVideo::restart() {
    if (!mf->reader) return openReader();

    const GUID nullGuid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
    PROPVARIANT v;
    PropVariantInit(&v);
    v.vt = VT_I8;
    v.hVal.QuadPart = 0;
    HRESULT hr = mf->reader->SetCurrentPosition(nullGuid, v);
    PropVariantClear(&v);

    if (FAILED(hr)) return openReader();   // can't seek: reopen from the start
    mf->needAnchor = true;
    return true;
}