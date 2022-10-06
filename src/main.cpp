/*
Application
*/

#include <random>

#include <grpcpp/server_builder.h>

#include "server.hpp"

#ifdef _WIN32
#define SLEEP(x) Sleep((x))
#else
#define SLEEP(x) sleep((x) / 1000)
#endif

mutex gMutex;
queue<WxMsg> gMsgQueue;
condition_variable gCv;

void ProduceMsg(queue<WxMsg> *msg_queue)
{
    random_device rd;  // 随机数生成器
    mt19937 gen(rd()); // Mersenne Twister 算法

    uniform_int_distribution<int> random_i(1000, 5000); // (1000, 5000) 均匀分布

    int i = 0;
    while (true) {
        int tmp = random_i(gen); // 生成 (1000, 5000) 随机数

        // 生成消息
        WxMsg msg;
        msg.set_is_self(false);
        msg.set_is_group(tmp % 2 == 0);
        msg.set_is_self(false);
        msg.set_is_group(tmp % 2 == 0);
        msg.set_type(tmp / 100);
        msg.set_id("Id_" + to_string(i++));
        msg.set_xml("<xml></xml>");
        msg.set_sender("wxid_" + to_string(tmp * tmp));
        msg.set_roomid(to_string(i * tmp) + "@chatroom");
        msg.set_content("Content" + to_string(tmp + tmp));

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

void ConsumeMsg(queue<WxMsg> *msg_queue)
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

int realSendTextMsg(string msg, string receiver, string aters)
{
    cout << "To[" << receiver << "]: " << msg << endl;
    return 0;
}

int realSendImageMsg(string path, string receiver)
{
    cout << "To[" << receiver << "]: " << path << endl;
    return 0;
}

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