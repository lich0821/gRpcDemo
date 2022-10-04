/*
Server
*/

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <string>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "../proto/demo.grpc.pb.h"

using namespace std;

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::Status;

using demo::Demo;
using demo::Empty;
using demo::Msg;

mutex gMutex;
queue<Msg> gMsgQueue;
condition_variable gCv;

#ifdef _WIN32
#define SLEEP(x) Sleep((x))
#else
#define SLEEP(x) sleep((x) / 1000)
#endif

void ProduceMsg(queue<Msg> *msg_queue)
{
    random_device rd;  // 随机数生成器
    mt19937 gen(rd()); // Mersenne Twister 算法

    uniform_int_distribution<int> random_i(1000, 5000); // (1000, 5000) 均匀分布

    int i = 0;
    while (true) {
        int tmp = random_i(gen); // 生成 (1000, 5000) 随机数

        // 生成消息
        Msg msg;
        msg.set_id("Id_" + to_string(i++));
        msg.set_type(to_string(tmp));

        // 推送到队列
        unique_lock<std::mutex> locker(gMutex);
        msg_queue->push(msg);
        locker.unlock();

        // 通知各方消息就绪
        gCv.notify_all();

        // 模拟消息到达间隔
        SLEEP(tmp);
    }
}

void ConsumeMsg(queue<Msg> *msg_queue)
{
    while (true) {
        unique_lock<std::mutex> lock(gMutex);
        gCv.wait(lock, [&] { return !msg_queue->empty(); });
        auto m = msg_queue->front();
        msg_queue->pop();
        lock.unlock();

        cout << m.id() << ": " << m.type() << endl;
    }
}

class DemoImpl final : public Demo::CallbackService
{
public:
    explicit DemoImpl(queue<Msg> &q, mutex &m, condition_variable &cv)
    {
        msg_q  = &q;
        msg_m  = &m;
        msg_cv = &cv;
    }

    grpc::ServerWriteReactor<Msg> *GetMessage(CallbackServerContext *context, const Empty *empty) override
    {
        class Getter : public grpc::ServerWriteReactor<Msg>
        {
        public:
            Getter(queue<Msg> *q, mutex *m, condition_variable *cv)
                : msg_q_(q)
                , msg_m_(m)
                , msg_cv_(cv)
            {
                cout << "New Call" << endl;
                NextWrite();
            }
            void OnDone() override { delete this; }
            void OnWriteDone(bool /*ok*/) override { NextWrite(); }

        private:
            void NextWrite()
            {
                unique_lock<std::mutex> lock(*msg_m_);
                gCv.wait(lock, [&] { return !msg_q_->empty(); });
                tmp_ = msg_q_->front();
                msg_q_->pop();
                lock.unlock();

                StartWrite(&tmp_);

                // Finish(Status::OK);  // 结束本次通信
            }
            Msg tmp_; // 如果将它放到 NextWrite 内部，StartWrite 调用时可能已经出了作用域
            queue<Msg> *msg_q_;
            mutex *msg_m_;
            condition_variable *msg_cv_;
        };

        return new Getter(msg_q, msg_m, msg_cv);
    }

private:
    queue<Msg> *msg_q;
    mutex *msg_m;
    condition_variable *msg_cv;
};

void RunServer()
{
    string server_address("localhost:50051");
    DemoImpl service(gMsgQueue, gMutex, gCv);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 2000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 3000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.RegisterService(&service);
    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << server_address << endl;
    server->Wait();
}

int main(int argc, char **argv)
{
    thread t1(ProduceMsg, &gMsgQueue); // 模拟生产消息
    // thread t2(ConsumeMsg, &gMsgQueue); // 验证消费消息

    RunServer();

    return 0;
}
