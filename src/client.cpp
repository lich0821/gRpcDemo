/*
RPC Client
*/

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

#include <memory>

#include <grpcpp/grpcpp.h>

#include "../proto/wcf.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using wcf::Wcf;
using wcf::Empty;
using wcf::WxMsg;

class DemoClient
{
public:
    DemoClient(shared_ptr<Channel> channel)
        : stub_(Wcf::NewStub(channel))
    {
    }

    void SetMsgHandleCb(function<void(WxMsg &)> msg_handle_cb) { GetMessage(msg_handle_cb); }

    void GetMessage(function<void(WxMsg &)> msg_handle_cb)
    {
        class Reader : public grpc::ClientReadReactor<WxMsg>
        {
        public:
            Reader(Wcf::Stub *stub, function<void(WxMsg &)> msg_handle_cb)
                : msg_handle_cb_(msg_handle_cb)
            {
                stub->async()->GetMessage(&context_, &empty_, this);
                StartRead(&msg_);
                StartCall();
            }

            void OnReadDone(bool ok) override
            {
                if (ok) {
                    try {
                        msg_handle_cb_(msg_);
                    } catch (...) {
                        cout << "OnMsg wrong..." << endl;
                    }
                    StartRead(&msg_);
                }
            }

            void OnDone(const Status &s) override
            {
                unique_lock<mutex> l(mu_);
                status_ = s;
                done_   = true;
                cv_.notify_one();
            }

            Status Await()
            {
                unique_lock<mutex> l(mu_);
                cv_.wait(l, [this] { return done_; });
                return move(status_);
            }

        private:
            wcf::Empty empty_;
            wcf::WxMsg msg_;
            ClientContext context_;

            mutex mu_;
            Status status_;
            bool done_ = false;
            condition_variable cv_;

            function<void(WxMsg &)> msg_handle_cb_;
        };

        Reader reader(stub_.get(), msg_handle_cb);
        Status status = reader.Await();

        if (!status.ok()) {
            cout << "GetMessage rpc failed." << endl;
        }
    }

private:
    unique_ptr<Wcf::Stub> stub_;
};

int OnMsg(WxMsg msg)
{
    cout << "Got Message: " << msg.is_self() << ", " << msg.is_group() << ", " << msg.type() << ", " << msg.id() << ", "
         << msg.xml() << ", " << msg.sender() << ", " << msg.roomid() << ", " << msg.content() << endl;
    return 0;
}

int main(int argc, char **argv)
{
    DemoClient wcf(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    function<void(WxMsg &)> cb = OnMsg;
    wcf.SetMsgHandleCb(cb);

    return 0;
}
