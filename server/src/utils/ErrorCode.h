#pragma once

namespace ErrorCode {
    constexpr int SUCCESS = 0;
    constexpr int BAD_REQUEST = 400;
    constexpr int UNAUTHORIZED = 401;
    constexpr int TOKEN_EXPIRED = 4011;
    constexpr int TOKEN_INVALID = 4012;
    constexpr int NOT_FOUND = 404;
    constexpr int INTERNAL_ERROR = 500;

    namespace User {
        constexpr int EMPTY_PARAMS = 1001;
        constexpr int USERNAME_TOO_LONG = 1002;
        constexpr int PASSWORD_TOO_SHORT = 1003;
        constexpr int USERNAME_EXISTS = 1004;
        constexpr int REGISTER_FAILED = 1005;
        constexpr int TOKEN_FAILED = 1006;
        constexpr int USER_NOT_FOUND = 1009;
        constexpr int WRONG_PASSWORD = 1010;
    }

    namespace Room {
        constexpr int INVALID_TOKEN = 2001;
        constexpr int EMPTY_TITLE = 2002;
        constexpr int ALREADY_LIVING = 2003;
        constexpr int CREATE_FAILED = 2004;
        constexpr int ROOM_NOT_FOUND = 2005;
        constexpr int NO_PERMISSION = 2006;
        constexpr int ROOM_ENDED = 2007;
        constexpr int END_FAILED = 2008;
        constexpr int USER_NOT_FOUND = 2009;
        constexpr int ALREADY_IN_ROOM = 2010;
    }

    namespace Replay {
        constexpr int NOT_FOUND = 4001;
    }
}
