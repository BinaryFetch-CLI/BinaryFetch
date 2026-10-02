// Video.h
// Windows-only. Streams frames from an ffmpeg pipe, Sixel-encoding one
// frame at a time (no pre-encoding, so long videos don't eat memory).
#pragma once
#include <string>
#include <vector>

class TerminalVideo {
public:
    TerminalVideo();
    ~TerminalVideo();
    TerminalVideo(const TerminalVideo&) = delete;
    TerminalVideo& operator=(const TerminalVideo&) = delete;

    void setPadding(int up, int left, int right);
    int  getPaddingUp() const;
    int  getPaddingLeft() const;
    int  getPaddingRight() const;
    int  getRowSpan() const;
    int  getColSpan() const;
    bool isLoaded() const;

    void setCellHeightPx(int px);
    void setCellWidthPx(int px);

    void setFps(int fps);          // clamped to 1..60
    int  getFps() const;
    void setFfmpegPath(const std::string& p);

    // Probes the video size, computes the scaled size, starts ffmpeg.
    bool load(const std::string& path, int sizePercent);
    // Reads + Sixel-encodes the next frame. False at end of video.
    bool nextFrame(std::string& outEncoded);
    // Restarts the stream from the beginning (used for looping).
    bool restart();
    void close();

private:
    bool startStream();

    int rowSpan, colSpan;
    int paddingUp, paddingLeft, paddingRight;
    int cellHeightPx, cellWidthPx;
    int fps;
    int outW, outH;
    bool loaded;
    std::string videoPath;
    std::string ffmpegPath;
    std::vector<unsigned char> rgba;
    void* hProcess;
    void* hPipe;
};