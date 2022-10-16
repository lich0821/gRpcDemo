#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import atexit
import logging
import re
from threading import Thread
from time import sleep
from typing import Callable, List, Optional

import grpc

import demo_pb2
import demo_pb2_grpc


class Demo():
    class WxMsg():
        def __init__(self, msg):
            self._is_self = msg.is_self
            self._is_group = msg.is_group
            self.type = msg.type
            self.id = msg.id
            self.xml = msg.xml
            self.sender = msg.sender
            self.roomid = msg.roomid
            self.content = msg.content

        def from_self(self):
            """是否自己发的消息"""
            return self._is_self == 1

        def from_group(self):
            """是否群聊消息"""
            return self._is_group

        def is_at(self, wxid):
            """群消息，在@名单里，并且不是@所有人"""
            return self.from_group() and re.findall(
                f"<atuserlist>.*({wxid}).*</atuserlist>", self.xml) and not re.findall(r"@(?:所有人|all)", self.xml)

        def is_text(self):
            """是否文本消息"""
            return self.type == 1

    def __init__(self, host_port: str = "localhost:10086") -> None:
        self._enable_recv_msg = False
        self.LOG = logging.getLogger("WCF")
        self._channel = grpc.insecure_channel(host_port)
        self._stub = demo_pb2_grpc.DemoStub(self._channel)
        atexit.register(self.disable_recv_msg)
        self.contacts = []
        self._SQL_TYPES = {
            0: lambda x: x.decode("utf-8"),
            1: lambda x: x.decode("utf-8"),
            2: lambda x: x.decode("utf-8"),
            3: lambda x: x.decode("utf-8"),
            4: lambda x: x.decode("utf-8")}

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
                func(self.WxMsg(rsp))
        except Exception as e:
            self.LOG.error(f"RpcEnableRecvMsg: {e}")
        finally:
            self.disable_recv_msg()

    def enable_recv_msg(self, callback: Callable[[WxMsg], None] = None) -> bool:
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

    def get_msg_types(self) -> dict:
        rsp = self._stub.RpcGetMsgTypes(demo_pb2.Empty())
        return dict(sorted(dict(rsp.types).items()))

    def get_contacts(self) -> List[dict]:
        rsp = self._stub.RpcGetContacts(demo_pb2.Empty())
        for cnt in rsp.contacts:
            gender = ""
            if cnt.gender == 1:
                gender = "男"
            elif cnt.gender == 2:
                gender = "女"
            self.contacts.append({"wxid": cnt.wxid, "code": cnt.code, "name": cnt.name,
                                 "country": cnt.country, "province": cnt.province, "city": cnt.city, "gender": gender})
        return self.contacts

    def get_dbs(self) -> List[str]:
        rsp = self._stub.RpcGetDbNames(demo_pb2.Empty())
        return rsp.names

    def get_tables(self, db: str) -> List[dict]:
        tables = []
        rsp = self._stub.RpcGetDbTables(demo_pb2.String(str=db))
        for tbl in rsp.tables:
            tables.append({"name": tbl.name, "sql": tbl.sql.replace("\t", "")})
        return tables

    def query_sql(self, db: str, sql: str) -> List[dict]:
        result = []
        rsp = self._stub.RpcExecDbQuery(demo_pb2.DbQuery(db=db, sql=sql))
        for r in rsp.rows:
            row = {}
            for f in r.fields:
                row[f.column] = self._SQL_TYPES[f.type](f.content)
            result.append(row)
        return result

    def accept_new_friend(self, v3: str, v4: str) -> int:
        rsp = self._stub.RpcAcceptNewFriend(demo_pb2.Verification(v3=v3, v4=v4))
        return rsp.status
