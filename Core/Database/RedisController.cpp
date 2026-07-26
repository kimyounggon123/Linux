#include "RedisController.hpp"

bool RedisController::Set(const std::string& key, const std::string& value, const uint64_t time)
{
    try
    {
        if (time == 0) redis.set(key.c_str(), value.c_str());
        else redis.set(key.c_str(), value.c_str(), std::chrono::seconds(time));
    }
    catch (const Error& e) 
    {
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
        return false;
    }
    return true;
}
bool RedisController::HashMapSet(const std::string& key, std::unordered_map<std::string, std::string> hashMap, const uint64_t time)
{
    try
    {
        redis.hmset(key, hashMap.begin(), hashMap.end());
        if (time != 0) redis.expire(key, std::chrono::seconds(time));
    }
    catch (const Error& e) 
    {
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
        return false;
    }
    return true;
}

const OptionalString RedisController::Get(const std::string& key)
{
    OptionalString retval;
    try
    {
        retval = redis.get(key.c_str());
    }
    catch (const Error& e) 
    {
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
        retval =  nullptr;
    }
    return retval;
}


bool RedisController::Exist(const std::string& key)
{
    bool retval;
    try
    {
        retval = redis.exists(key.c_str());
    }
    catch (const Error& e) 
    {
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
        retval = false;
    }
    return retval;
}

bool RedisController::Delete(const std::string& key)
{
    bool retval;
    try
    {
        retval = redis.del(key.c_str());
    }
    catch (const Error& e) 
    {
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
        retval = false;
    }
    return retval;   
}
void RedisController::flushall()
{
    redis.flushall();
}
/*
void RedisBasicTutorial()
{
    try {
        // 1. 🔗 Redis 서버 연결 (IP, Port 세팅)
        // 만약 비밀번호가 있다면 "tcp://:비밀번호@127.0.0.1:6379" 형태로 작성합니다.
        auto redis = Redis("tcp://127.0.0.1:6379");

        // 2. 📥 데이터 저장 (SET)
        // 일반적인 문자열 키-값 저장입니다.
        redis.set("LATEST_PLAYER_ID", "99");


        // 3. ⏳ 만료 시간과 함께 저장 (SET with TTL) ──⭐ [토큰 인증의 핵심]
        // 게임 서버에서 가장 중요한 기능입니다. 특정 시간 뒤에 데이터가 자동으로 파괴됩니다.
        // 아래 코드는 "TOKEN:abcde"라는 키를 10초 동안만 유지하겠다는 뜻입니다.
        redis.set("TOKEN:abcde", "Player_99", std::chrono::seconds(10));


        // 4. 📤 데이터 조회 (GET)
        // redis-plus-plus의 get()은 데이터가 없을 가능성을 고려해 
        // C++17의 'std::optional<std::string>' 타입을 반환합니다.
        auto val = redis.get("TOKEN:abcde");

        if (val) {
            // 값이 존재하면 포인터처럼 * 연산자로 알맹이를 꺼냅니다.
            std::cout << "토큰 조회 성공! 유저: " << *val << std::endl;
        } else {
            // 10초가 지났거나 없는 키인 경우 이쪽으로 들어옵니다.
            std::cout << "만료되었거나 존재하지 않는 토큰입니다." << std::endl;
        }


        // 5. ❌ 데이터 수동 삭제 (DEL)
        // 1회용 토큰 검증이 끝났거나 유저가 로그아웃했을 때 데이터를 파괴합니다.
        bool isDeleted = redis.del("LATEST_PLAYER_ID"); // 성공 시 true(1) 반환
        if (isDeleted) {
            std::cout << "데이터가 성공적으로 삭제되었습니다." << std::endl;
        }


        // 6. 🔍 데이터 존재 여부 확인 (EXISTS)
        if (redis.exists("TOKEN:abcde")) {
            std::cout << "아직 토큰이 살아있습니다." << std::endl;
        } else {
            std::cout << "토큰이 없습니다." << std::endl;
        }

    }
    catch (const Error& e) {
        // Redis 서버가 꺼져있거나 네트워크가 단절되면 예외가 발생합니다.
        // 서버 크래시를 막기 위해 반드시 try-catch로 감싸주어야 합니다.
        std::cerr << "Redis 작업 중 예외 발생: " << e.what() << std::endl;
    }
}
*/