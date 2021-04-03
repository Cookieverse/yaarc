#ifndef YAARC_YAARC_TEST_HELPER_HPP
#define YAARC_YAARC_TEST_HELPER_HPP
#include <stdexcept>
#define TEST_ERROR(error) std::cout << (error) << " in " << __FILE__ << ":" << __LINE__ << std::endl; exit(1)
#define TEST_ASSERT(value, expected) if (value != expected) {std::cout << (fmt::format("Expected {}, but got {} in {}:{}", expected, value, __FILE__, __LINE__)) << std::endl; exit(1);}
#define TEST_ASSERT_HINT(hint, value, expected) if (value != expected) {std::cout << (fmt::format("Expected {}, but got {} during check {} in {}:{}", expected, value, hint, __FILE__, __LINE__)) << std::endl; exit(1);}
#define TEST_ASSERT_UNEQUAL(value, expected) if (value == expected) {std::cout << (fmt::format("Expected {} to not match {}, but they did in {}:{}", expected, value, __FILE__, __LINE__)) << std::endl; exit(1);}
#define TEST_ASSERT_UNEQUAL_HINT(hint, value, expected) if (value == expected) {std::cout << (fmt::format("Expected {} to not match {}, but they did during check {} in {}:{}", expected, value, hint, __FILE__, __LINE__)) << std::endl; exit(1);}
#endif //YAARC_YAARC_TEST_HELPER_HPP
