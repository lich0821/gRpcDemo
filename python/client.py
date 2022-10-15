#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import atexit
import logging
from threading import Thread
from time import sleep
from typing import Any, Callable, Optional

import grpc

import demo_pb2
import demo_pb2_grpc


class Demo():
    def __init__(self, host_port: str = "localhost:10086") -> None:
        self._enable_recv_msg = False
        self.LOG = logging.getLogger("WCF")
        self._channel = grpc.insecure_channel(host_port)
        self._stub = demo_pb2_grpc.DemoStub(self._channel)
        atexit.register(self.disable_recv_msg)

    def __del__(self):
        self.disable_recv_msg()
        self._channel.close()

    def keep_running(self):
        while True:
            sleep(10)

    def is_login(self) -> int:
        rsp = self._stub.RpcIsLogin(demo_pb2.Empty())
        return rsp.status

    def get_self_wxid(self) -> str:
        rsp = self._stub.RpcGetSelfWxid(demo_pb2.Empty())
        return rsp.str

    def _rpc_get_message(self, func):
        rsps = self._stub.RpcEnableRecvMsg(demo_pb2.Empty())
        try:
            for rsp in rsps:
                func(rsp)
        except Exception as e:
            self.LOG.error(f"RpcEnableRecvMsg: {e}")
        finally:
            self.disable_recv_msg()

    def enable_recv_msg(self, callback: Callable[..., Any] = None) -> bool:
        if self._enable_recv_msg:
            return True

        if callback is None:
            return False

        self._enable_recv_msg = True
        # 阻塞，把控制权交给用户
        # self._rpc_get_message(callback)

        # 不阻塞，启动一个新的线程来接收消息
        Thread(target=self._rpc_get_message, name="GetMessage", args=(callback,), daemon=True).start()

        return True

    def disable_recv_msg(self) -> int:
        if not self._enable_recv_msg:
            return -1

        rsp = self._stub.RpcDisableRecvMsg(demo_pb2.Empty())
        if rsp.status == 0:
            self._enable_recv_msg = False

        return rsp.status

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        rsp = self._stub.RpcSendTextMsg(demo_pb2.TextMsg(msg=msg, receiver=receiver, aters=aters))
        return rsp.status

    def send_image(self, path: str, receiver: str) -> int:
        rsp = self._stub.RpcSendImageMsg(demo_pb2.ImageMsg(path=path, receiver=receiver))
        return rsp.status

    def get_msg_types(self) -> demo_pb2.MsgTypes:
        rsp = self._stub.RpcGetMsgTypes(demo_pb2.Empty())
        return rsp

    def get_contacts(self) -> demo_pb2.Contacts:
        rsp = self._stub.RpcGetContacts(demo_pb2.Empty())
        return rsp

    def get_dbs(self) -> demo_pb2.DbNames:
        rsp = self._stub.RpcGetDbNames(demo_pb2.Empty())
        return rsp

    def get_tables(self, db: str) -> demo_pb2.DbTables:
        rsp = self._stub.RpcGetDbTables(demo_pb2.String(str=db))
        return rsp

    def query_sql(self, db: str, sql: str) -> demo_pb2.DbRows:
        rsp = self._stub.RpcExecDbQuery(demo_pb2.DbQuery(db=db, sql=sql))
        return rsp

    def accept_new_friend(self, v3: str, v4: str) -> int:
        rsp = self._stub.RpcAcceptNewFriend(demo_pb2.Verification(v3=v3, v4=v4))
        return rsp.status
