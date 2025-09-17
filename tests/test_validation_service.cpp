#include <catch2/catch_test_macros.hpp>
#include "auth/validation_service.h"

TEST_CASE("ValidationService Email Tests", "[validation][email]") {
    auth::ValidationService validator;

    SECTION("Valid email formats") {
        // Standard valid emails
        REQUIRE_FALSE(validator.validateEmail("test@example.com").has_value());
        REQUIRE_FALSE(validator.validateEmail("user@domain.org").has_value());
        REQUIRE_FALSE(validator.validateEmail("name@company.co.uk").has_value());

        // Complex but valid emails
        REQUIRE_FALSE(validator.validateEmail("user.name+tag@example.com").has_value());
        REQUIRE_FALSE(validator.validateEmail("user_name@example-domain.com").has_value());
        REQUIRE_FALSE(validator.validateEmail("123@example.com").has_value());
        REQUIRE_FALSE(validator.validateEmail("a@b.co").has_value());
    }

    SECTION("Invalid email formats") {
        // Empty email
        auto error = validator.validateEmail("");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Email cannot be empty");

        // Missing @ symbol
        error = validator.validateEmail("testexample.com");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        // Missing domain
        error = validator.validateEmail("test@");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        // Missing username
        error = validator.validateEmail("@example.com");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        // No domain extension
        error = validator.validateEmail("test@domain");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        // Invalid characters
        error = validator.validateEmail("test user@example.com");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        // Multiple @ symbols
        error = validator.validateEmail("test@@example.com");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");
    }
}

TEST_CASE("ValidationService Password Tests", "[validation][password]") {
    auth::ValidationService validator;

    SECTION("Valid passwords") {
        REQUIRE_FALSE(validator.validatePassword("password123").has_value());
        REQUIRE_FALSE(validator.validatePassword("123456").has_value());
        REQUIRE_FALSE(validator.validatePassword("mypassword").has_value());
        REQUIRE_FALSE(validator.validatePassword("P@ssw0rd!").has_value());
        REQUIRE_FALSE(validator.validatePassword("very_long_password_that_is_secure").has_value());
    }

    SECTION("Invalid passwords") {
        // Empty password
        auto error = validator.validatePassword("");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Password cannot be empty");

        // Too short passwords
        error = validator.validatePassword("12345");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Password must be at least 6 characters");

        error = validator.validatePassword("a");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Password must be at least 6 characters");

        error = validator.validatePassword("abc");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Password must be at least 6 characters");
    }
}

TEST_CASE("ValidationService Username Tests", "[validation][username]") {
    auth::ValidationService validator;

    SECTION("Valid usernames") {
        REQUIRE_FALSE(validator.validateUsername("user123").has_value());
        REQUIRE_FALSE(validator.validateUsername("test_user").has_value());
        REQUIRE_FALSE(validator.validateUsername("ABC").has_value());
        REQUIRE_FALSE(validator.validateUsername("player1").has_value());
        REQUIRE_FALSE(validator.validateUsername("my_game_name").has_value());
        REQUIRE_FALSE(validator.validateUsername("User_123").has_value());
    }

    SECTION("Invalid usernames") {
        // Empty username
        auto error = validator.validateUsername("");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username cannot be empty");

        // Too short usernames
        error = validator.validateUsername("ab");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username must be at least 3 characters");

        error = validator.validateUsername("a");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username must be at least 3 characters");

        // Too long username
        error = validator.validateUsername("this_is_a_very_long_username_that_exceeds_the_limit");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username must be less than 20 characters");

        // Invalid characters
        error = validator.validateUsername("user@name");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");

        error = validator.validateUsername("user name");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");

        error = validator.validateUsername("user-name");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");

        error = validator.validateUsername("user.name");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");

        error = validator.validateUsername("user#name");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");
    }
}

TEST_CASE("ValidationService Password Confirmation Tests", "[validation][password_confirmation]") {
    auth::ValidationService validator;

    SECTION("Matching passwords") {
        REQUIRE_FALSE(validator.validatePasswordConfirmation("password", "password").has_value());
        REQUIRE_FALSE(validator.validatePasswordConfirmation("123456", "123456").has_value());
        REQUIRE_FALSE(validator.validatePasswordConfirmation("", "").has_value());
        REQUIRE_FALSE(validator.validatePasswordConfirmation("P@ssw0rd!", "P@ssw0rd!").has_value());
    }

    SECTION("Non-matching passwords") {
        auto error = validator.validatePasswordConfirmation("password", "different");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Passwords do not match");

        error = validator.validatePasswordConfirmation("123456", "654321");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Passwords do not match");

        error = validator.validatePasswordConfirmation("password", "");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Passwords do not match");

        error = validator.validatePasswordConfirmation("", "password");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Passwords do not match");
    }
}

TEST_CASE("ValidationService Login Credentials Tests", "[validation][login_credentials]") {
    auth::ValidationService validator;

    SECTION("Valid login credentials") {
        REQUIRE_FALSE(validator.validateLoginCredentials("user", "pass").has_value());
        REQUIRE_FALSE(validator.validateLoginCredentials("testuser", "password123").has_value());
        REQUIRE_FALSE(validator.validateLoginCredentials("a", "b").has_value()); // Minimal but complete
    }

    SECTION("Invalid login credentials") {
        auto error = validator.validateLoginCredentials("", "pass");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please enter username and password");

        error = validator.validateLoginCredentials("user", "");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please enter username and password");

        error = validator.validateLoginCredentials("", "");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please enter username and password");
    }
}

TEST_CASE("ValidationService Registration Data Tests", "[validation][registration_data]") {
    auth::ValidationService validator;

    SECTION("Valid registration data") {
        REQUIRE_FALSE(validator.validateRegistrationData(
            "user123", "test@example.com", "password123", "password123").has_value());
        REQUIRE_FALSE(validator.validateRegistrationData(
            "testuser", "user@domain.org", "mypassword", "mypassword").has_value());
        REQUIRE_FALSE(validator.validateRegistrationData(
            "abc", "a@b.co", "123456", "123456").has_value());
    }

    SECTION("Invalid registration data - empty fields") {
        auto error = validator.validateRegistrationData("", "test@example.com", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please fill in all fields");

        error = validator.validateRegistrationData("user123", "", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please fill in all fields");

        error = validator.validateRegistrationData("user123", "test@example.com", "", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please fill in all fields");

        error = validator.validateRegistrationData("user123", "test@example.com", "password123", "");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Please fill in all fields");
    }

    SECTION("Invalid registration data - username issues") {
        auto error = validator.validateRegistrationData("ab", "test@example.com", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username must be at least 3 characters");

        error = validator.validateRegistrationData("user@name", "test@example.com", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Username can only contain letters, numbers, and underscores");
    }

    SECTION("Invalid registration data - email issues") {
        auto error = validator.validateRegistrationData("user123", "invalid-email", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");

        error = validator.validateRegistrationData("user123", "test@", "password123", "password123");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Invalid email format");
    }

    SECTION("Invalid registration data - password issues") {
        auto error = validator.validateRegistrationData("user123", "test@example.com", "12345", "12345");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Password must be at least 6 characters");

        error = validator.validateRegistrationData("user123", "test@example.com", "password123", "different");
        REQUIRE(error.has_value());
        REQUIRE(error.value() == "Passwords do not match");
    }
}

TEST_CASE("ValidationService Edge Cases", "[validation][edge_cases]") {
    auth::ValidationService validator;

    SECTION("Whitespace handling") {
        // Emails with whitespace (should be invalid)
        auto error = validator.validateEmail(" test@example.com");
        REQUIRE(error.has_value());

        error = validator.validateEmail("test@example.com ");
        REQUIRE(error.has_value());

        error = validator.validateEmail("test @example.com");
        REQUIRE(error.has_value());

        // Usernames with whitespace (should be invalid)
        error = validator.validateUsername(" username");
        REQUIRE(error.has_value());

        error = validator.validateUsername("username ");
        REQUIRE(error.has_value());

        error = validator.validateUsername("user name");
        REQUIRE(error.has_value());
    }

    SECTION("Unicode and special characters") {
        // Unicode in username (should be invalid)
        auto error = validator.validateUsername("usér");
        REQUIRE(error.has_value());

        error = validator.validateUsername("用户");
        REQUIRE(error.has_value());

        // Special characters in username
        error = validator.validateUsername("user!");
        REQUIRE(error.has_value());

        error = validator.validateUsername("user$");
        REQUIRE(error.has_value());
    }

    SECTION("Very long inputs") {
        // Very long email
        std::string long_email = std::string(100, 'a') + "@example.com";
        // This should still validate by format, but might be practically too long

        // Very long password (should be valid)
        std::string long_password(1000, 'a');
        REQUIRE_FALSE(validator.validatePassword(long_password).has_value());
    }
}