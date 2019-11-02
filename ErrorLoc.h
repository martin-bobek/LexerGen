#ifndef ERROR_LOC_H__
#define ERROR_LOC_H__

#define ERROR_LOC() ErrorLoc(__FILE__, __func__, __LINE__)

struct ErrorLoc {
    ErrorLoc(const char *file_, const char *func_, int line_) noexcept :
        File(file_), Function(func_), Line(line_) {}

    const char *File;
    const char *Function;
    const int Line;
};

#endif
