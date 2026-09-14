#ifndef REDISCONTROLLER_H
#define REDISCONTROLLER_H

#include <iostream>
#include <sw/redis++/redis++.h>
using namespace sw::redis;
class RedisControl
{
    std::unique_ptr<Redis> redis;
public:
    RedisControl(): redis(nullptr) 
    {}
    ~RedisControl()
    {
        flushall();
    }

    bool Connect(const char* addr = "127.0.0.1", int port = 6379);
    bool Set(const std::string& key, const std::string& value, const uint64_t time = 0);
    bool HashMapSet(const std::string& key, std::unordered_map<std::string, std::string> hashMap, const uint64_t time = 0);
    
    bool Exist(const std::string& key);

    const OptionalString Get(const std::string& key);
    
    bool Delete(const std::string& key);
    void flushall(); // redis 내의 모든 데이터 삭제

    std::string IntToString(const int data) {return std::to_string(data);}
    int32_t StringToInt(const std::string& data) {return atoi(data.c_str());}
};
#endif

/*


설치가 완료되면 내 소스 코드에서 헤더를 인클루드하고, 
컴파일러(g++)에게 hiredis와 redis-plus-plus 라이브러리를 링크(-l)해 달라고 명시해야 합니다.

g++로 직접 빌드할 때는 순서가 중요합니다.
redis++가 hiredis에 의존하기 때문에 -lredis++를 먼저 적고 뒤에 -lhiredis를 적어야 링킹 에러가 안 납니다.

=> g++ -std=c++17 main.cpp -o redis_test -lredis++ -lhiredis


# 정석: Redis 전용 도구로 안전하게 종료 (메모리 데이터를 디스크에 저장하고 꺼짐)
redis-cli shutdown

# 만약 먹통이 되어서 강제 종료해야 할 때 (프로세스 킬)
sudo systemctl stop redis-server
# 또는
sudo pkill -9 redis-server


키기
sudo service redis-server start


끄기
sudo service redis-server stop


상태 확인
sudo service redis-server status

재시작
sudo service redis-server restart
*/