/*
 * Copyright (C) 2024, 2025  Dicot0721
 *
 * This file is part of Android-Memory-Editor.
 *
 * Android-Memory-Editor is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Android-Memory-Editor is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Android-Memory-Editor.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AME_FILE_H
#define AME_FILE_H

#include <fcntl.h>
#include <unistd.h>

#include <cstddef>

#include <string_view>
#include <utility>

namespace ame {

class FileWrapper {
public:
    FileWrapper(std::string_view file, int oflag)
        : _fd{open64(file.data(), oflag)} {}

    explicit FileWrapper(std::string_view file)
        : FileWrapper{file, O_RDWR} {}

    FileWrapper(const FileWrapper &) = delete;

    FileWrapper(FileWrapper &&other) noexcept
        : _fd{other._fd} {
        other._fd = -1;
    };

    ~FileWrapper() {
        if (IsOpen()) {
            close(_fd);
        }
    }

    FileWrapper &operator=(const FileWrapper &) = delete;

    FileWrapper &operator=(FileWrapper &&other) noexcept {
        std::swap(_fd, other._fd);
        return *this;
    }

    [[nodiscard]] bool IsOpen() const noexcept {
        return _fd != -1;
    }

    /**
     * @brief Read NBYTES into BUF from FD at the given position OFFSET without changing the file pointer.
     * @return The number read.
     * @retval -1  for errors.
     * @retval 0  for EOF.
     */
    ssize_t PRead64(void *buf, std::size_t nbytes, off64_t offset) {
        return pread64(_fd, buf, nbytes, offset);
    }

    /**
     * @brief Write N bytes of BUF to FD at the given position OFFSET without changing the file pointer.
     * @return The number written, or -1.
     */
    ssize_t PWrite64(const void *buf, std::size_t n, off64_t offset) {
        return pwrite64(_fd, buf, n, offset);
    }

protected:
    int _fd = -1;
};

} // namespace ame

#endif // AME_FILE_H
