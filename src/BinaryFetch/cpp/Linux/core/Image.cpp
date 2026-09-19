// Image.cpp (Linux stub/dummy implementation)
#include "core/Image.h"

TerminalImage::TerminalImage()
    : rowSpan(0), colSpan(0), paddingUp(0), paddingLeft(0),
      paddingRight(0), cellHeightPx(0), cellWidthPx(0), loaded(false) {}

bool TerminalImage::load(const std::string& /*path*/, int /*sizePercent*/) {
    // Image support is a placeholder on Linux for now
    loaded = false;
    return false;
}

void TerminalImage::draw() const {
    // No-op on Linux for now
}

bool TerminalImage::isLoaded() const {
    return false;
}

int TerminalImage::getRowSpan() const {
    return rowSpan;
}

int TerminalImage::getColSpan() const {
    return colSpan;
}

void TerminalImage::setPadding(int up, int left, int right) {
    paddingUp = up;
    paddingLeft = left;
    paddingRight = right;
}

int TerminalImage::getPaddingUp() const {
    return paddingUp;
}

int TerminalImage::getPaddingLeft() const {
    return paddingLeft;
}

int TerminalImage::getPaddingRight() const {
    return paddingRight;
}

void TerminalImage::setCellHeightPx(int px) {
    cellHeightPx = px;
}

int TerminalImage::getCellHeightPx() const {
    return cellHeightPx;
}

void TerminalImage::setCellWidthPx(int px) {
    cellWidthPx = px;
}

int TerminalImage::getCellWidthPx() const {
    return cellWidthPx;
}
