#! /usr/bin/env python3
# -*- coding: utf-8 -*-

from client import Demo


def func(msg: Demo.WxMsg):
    if msg.from_group():
        s = f"【{msg.roomid}】的 {msg.sender} {'@你'if msg.is_at('me') else '发来的消息'}：\n"
    else:
        s = f"好友 {msg.sender} 发来的消息：\n"

    s += msg.content
    print(s)


def main():
    wcf = Demo()
    print(f"Is Login: {True if wcf.is_login() else False}")
    print(f"SelfWxid: {wcf.get_self_wxid()}")

    wcf.enable_recv_msg(func)
    # wcf.disable_recv_msg() # Call anytime when you don't want to receive messages
    wcf.send_text("Hello world.", "wxid_to_send")
    wcf.send_image("image path", "yuwangdadi")

    print(f"Message types:\n{wcf.get_msg_types()}")
    print(f"Contacts:\n{wcf.get_contacts()}")

    print(f"DBs:\n{wcf.get_dbs()}")
    print(f"Tables:\n{wcf.get_tables('db')}")
    print(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    wcf.accept_new_friend("v3", "v4")

    # Keep running to receive messages
    wcf.keep_running()


if __name__ == "__main__":
    main()
