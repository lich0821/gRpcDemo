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

#include "demo.grpc.pb.h"

using namespace std;

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerUnaryReactor;
using grpc::ServerWriteReactor;
using grpc::Status;

using demo::Contact;
using demo::Contacts;
using demo::DbField;
using demo::DbNames;
using demo::DbQuery;
using demo::DbRow;
using demo::DbRows;
using demo::DbTable;
using demo::DbTables;
using demo::Demo;
using demo::Empty;
using demo::ImageMsg;
using demo::MsgTypes;
using demo::Response;
using demo::String;
using demo::TextMsg;
using demo::Verification;
using demo::WxMsg;

extern int realIsLogin();
extern string realGetSelfWxid();
extern int realSendTextMsg(string msg, string receiver, string aters);
extern int realSendImageMsg(string path, string receiver);
extern bool realGetMsgTypes(MsgTypes *types);
extern bool realGetContacts(Contacts *contacts);
extern bool realGetDbNames(DbNames *names);
extern bool realGetDbTables(const string db, DbTables *tables);
extern bool realExecDbQuery(const string db, const string sql, DbRows *rows);
extern bool realAcceptNewFriend(const string v3, const string v4);

mutex gMutex;
queue<WxMsg> gMsgQueue;
condition_variable gCv;
bool gIsListening;

class DemoImpl final : public Demo::CallbackService
{
public:
    explicit DemoImpl() { }
    ServerUnaryReactor *RpcIsLogin(CallbackServerContext *context, const Empty *empty, Response *rsp) override
    {
        int ret = realIsLogin();
        rsp->set_status(ret);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcGetSelfWxid(CallbackServerContext *context, const Empty *empty, String *rsp) override
    {
        string wxid = realGetSelfWxid();
        rsp->set_str(wxid);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerWriteReactor<WxMsg> *RpcEnableRecvMsg(CallbackServerContext *context, const Empty *empty) override
    {
        class Getter : public ServerWriteReactor<WxMsg>
        {
        public:
            Getter()
            {
                cout << "New Call" << endl;
                NextWrite();
            }
            void OnDone() override { delete this; }
            void OnWriteDone(bool /*ok*/) override { NextWrite(); }

        private:
            void NextWrite()
            {
                // unique_lock<std::mutex> lock(*msg_m_);
                // msg_cv_->wait(lock, [&] { return !msg_q_->empty(); });
                // tmp_ = msg_q_->front();
                // msg_q_->pop();
                // lock.unlock();

                unique_lock<std::mutex> lock(gMutex);
                gCv.wait(lock, [&] { return !gMsgQueue.empty(); });
                tmp_ = gMsgQueue.front();
                gMsgQueue.pop();
                lock.unlock();

                StartWrite(&tmp_);

                // Finish(Status::OK);  // 结束本次通信
            }
            WxMsg tmp_; // 如果将它放到 NextWrite 内部，StartWrite 调用时可能已经出了作用域
        };

        return new Getter();
    }

    ServerUnaryReactor *RpcDisableRecvMsg(CallbackServerContext *context, const Empty *empty, Response *rsp) override
    {
        if (gIsListening) {
            gIsListening = false;
            // 发送消息，触发 NextWrite 的 Finish
            WxMsg wxMsg;
            unique_lock<std::mutex> lock(gMutex);
            gMsgQueue.push(wxMsg);
            lock.unlock();
            gCv.notify_all();
        }

        rsp->set_status(0);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcSendTextMsg(CallbackServerContext *context, const TextMsg *msg, Response *rsp) override
    {
        int ret = realSendTextMsg(msg->msg(), msg->receiver(), msg->aters());
        rsp->set_status(ret);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcSendImageMsg(CallbackServerContext *context, const ImageMsg *msg, Response *rsp) override
    {
        int ret = realSendImageMsg(msg->path(), msg->receiver());
        rsp->set_status(ret);
        rsp->set_status(0);
        auto *reactor = context->DefaultReactor();
        reactor->Finish(Status::OK);
        return reactor;
    }

    ServerUnaryReactor *RpcGetMsgTypes(CallbackServerContext *context, const Empty *empty, MsgTypes *rsp) override
    {
        bool ret      = realGetMsgTypes(rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcGetContacts(CallbackServerContext *context, const Empty *empty, Contacts *rsp) override
    {
        bool ret      = realGetContacts(rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcGetDbNames(CallbackServerContext *context, const Empty *empty, DbNames *rsp) override
    {
        bool ret      = realGetDbNames(rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcGetDbTables(CallbackServerContext *context, const String *db, DbTables *rsp) override
    {
        bool ret      = realGetDbTables(db->str(), rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcExecDbQuery(CallbackServerContext *context, const DbQuery *query, DbRows *rsp) override
    {
        bool ret      = realExecDbQuery(query->db(), query->sql(), rsp);
        auto *reactor = context->DefaultReactor();
        if (ret) {
            reactor->Finish(Status::OK);
        } else {
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }

    ServerUnaryReactor *RpcAcceptNewFriend(CallbackServerContext *context, const Verification *v,
                                           Response *rsp) override
    {
        bool ret      = realAcceptNewFriend(v->v3(), v->v4());
        auto *reactor = context->DefaultReactor();
        if (ret) {
            rsp->set_status(0);
            reactor->Finish(Status::OK);
        } else {
            rsp->set_status(-1); // TODO: Unify error code
            reactor->Finish(Status::CANCELLED);
        }

        return reactor;
    }
};
