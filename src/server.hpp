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
                msg_cv_->wait(lock, [&] { return !msg_q_->empty(); });
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
