// Video.h
// Windows-only. Decodes via Windows Media Foundation (built into Windows,
// no external dependency) and Sixel-encodes one frame at a time.
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

    void setFps(int fps);          // clamped to 1..60 (and to the video's own fps)
    int  getFps() const;
    void setFlipVertical(bool f);  // use if the video appears upside down

    bool load(const std::string& path, int sizePercent);
    bool nextFrame(std::string& outEncoded);   // false at end of video
    bool restart();                            // back to the start (looping)
    void close();

private:
    struct MFState;
    bool openReader();

    int rowSpan, colSpan;
    int paddingUp, paddingLeft, paddingRight;
    int cellHeightPx, cellWidthPx;
    int fps;
    int outW, outH;
    bool loaded;
    bool flipVertical;
    std::string videoPath;
    std::vector<unsigned char> rgba;
    MFState* mf;
};