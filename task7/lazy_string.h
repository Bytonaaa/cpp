#ifndef TASK6_LAZY_STRING_H
#define TASK6_LAZY_STRING_H

#include <string>
#include <istream>
#include <ostream>
#include <memory>
#include <mutex>

/**
 * The lazy_string class implements Copy-on-Write string
 */

class lazy_string {
    struct char_ref {
        friend class lazy_string;

        operator char() const;
        char_ref &operator=(char);

    private:
        char_ref(lazy_string *, size_t);

        const size_t index;
        lazy_string *const ls;
    };

public:
    /**
     * Creates a std::string with a copy of the content of the current lazy_string.
     *
     * @return  A std::string containing a copy of the characters of the current lazy_string.
     */
    operator std::string();

    /**
     * Constructs empty lazy_string (zero size and unspecified capacity).
     */
    lazy_string();

    /**
     * Constructs new lazy_string from a std::string.
     *
     * @param   str A std::string.
     */
    lazy_string(const std::string &str);

    /**
     * Constructs new lazy_string from a lazy_string.
     * Does not copy the data.
     *
     * @param   ls A lazy_string
     */
    lazy_string(const lazy_string &ls);

    /**
     * Returns the number of characters in the string
     *
     * @return The number of characters in the string.
     */
    size_t size() const;

    /**
     * Returns the number of characters in the string
     *
     * @return The number of characters in the string.
     */
    size_t length() const;

    /**
     * Returns a substring [pos, pos+count).
     * If the requested substring extends past the end of the string,
     * or if count == npos, the returned substring is [pos, size()).
     * Does not copy the buffer of the lazy_string.
     *
     * @param   pos position of the first character to include
     * @param   len length of the substring
     *
     * @return  1) An empty string if pos == size().
     * @return  2) lazy_string containing the substring [pos, pos+count).
     *
     * @throws  std::out_of_range if pos > size()
     */
    lazy_string substr(size_t pos = 0, size_t len = std::string::npos);

    /**
     * Returns a reference to the character at specified location pos.
     * Bounds checking is performed, exception of type std::out_of_range
     * will be thrown on invalid access.
     *
     * @param   pos position of the character to return
     *
     * @return  Reference to the requested character.
     *
     * @throws  std::out_of_range if pos >= size().
     */
    char_ref at(size_t pos);
    char at(size_t pos) const;

    /**
     * Returns a reference to the character at specified location pos.
     * No bounds checking is performed.
     *
     * If pos == size(), a reference to the null character is returned.
     * For the first (non-const) version, the behavior is undefined if this character is modified.
     *
     * @param   pos position of the character to return
     *
     * @return  Reference to the requested character.
     *
     * @throws  std::out_of_range if pos >= size().
     */
    char_ref operator[](size_t pos);
    char operator[](size_t pos) const;

    /**
     * Replaces the contents of the string.
     *
     * @param   ls A lazy_string to be used as source to initialize the string with
     *
     * @return  *this
     */
    lazy_string &operator=(const lazy_string &ls);

    /**
     * Extracts a string from the input stream is, storing the sequence in ls,
     * which is overwritten (the previous value of ls is replaced).
     *
     * @param   is  std::istream object from which characters are extracted.
     * @param   ls  lazy_string object where the extracted content is stored.
     *
     * @return  The same as parameter is.
     */
    friend std::istream &operator>>(std::istream &is, lazy_string &ls);

    /**
     * Inserts the sequence of characters that conforms value of str into os.
     *
     * @param   os  std::ostream object where characters are inserted.
     * @param   ls  lazy_string object with the content to insert.
     *
     * @return  The same as parameter os.
     */
    friend std::ostream &operator<<(std::ostream &os, lazy_string &ls);

private:
    size_t start, sz;
    std::shared_ptr<std::string> ref;
    std::mutex mtx;
};

#endif //TASK6_LAZY_STRING_H
