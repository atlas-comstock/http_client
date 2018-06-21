# http_client
A http client written in C and pure socket, for understanding HTTP protocol. 用于理解 http 协议的 http 客户端

我们想想浏览器做了什么事情, 根据 url, 请求对方服务器, 获取相应的文件.

1. 根据输入的 url 地址, 解析出 hostname. 如 根据`https://stackoverflow.com/questions/tagged/elixir`, hostname 为 stackoverflow.com, 其他的不过是 URL 的一部分. 程序中的extract_hostname 函数便是做这个事情
2. 把 hostname 解析成 ip 地址,  我的函数·`getIPFromDNS` 便是做这个事情, 主要调用 linux 的 [gethostbyname](http://man7.org/linux/man-pages/man3/gethostbyname.3.html) 即可解析dns, 得到一个ip 数组, 通常选一个即可.
3.  我的函数`init_serv_addr`配置 socket 的信息, 如使用 `ipv4`, 用80端口, 访问哪个 ip .
4. 连接 socket, `generate_request_header` 生成 http request 头部,  注意第一行即描述了使用GET 协议, HTTP 1.1 版本, `HOST` 是必须的, 因为大多数 web 服务器都设置了虚拟主机, 也就是根据 HOST 来 redirect 你到不同的 地方.
5. 收对方的回复, 收到response 的头部后(根据\r\n\r\n 划分), 解析出 Content-Length, 接着收剩下的内容~
至此, 一个简单的 http client 完成.

示例：
http://example.com
 解析到的 ip 地址为: `IP ADDRESS->93.184.216.34`

-> HTTP请求报文如下
```
--------HTTP Request--------
GET / HTTP/1.1
HOST: example.com
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36
Cache-Control: no-cache
```

 服务器response回复的头部为：
```
HTTP/1.1 200 OK
Cache-Control: max-age=604800
Content-Type: text/html
Date: Thu, 21 Jun 2018 10:35:29 GMT
Etag: "1541025663+ident"
Expires: Thu, 28 Jun 2018 10:35:29 GMT
Last-Modified: Fri, 09 Aug 2013 23:54:35 GMT
Server: ECS (oxr/8313)
Vary: Accept-Encoding
X-Cache: HIT
Content-Length: 1270
```
接着的内容是：
```
<!doctype html>
<html>
<head>
    <title>Example Domain</title>

    <meta charset="utf-8" />
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <style type="text/css">
    body {
        background-color: #f0f0f2;
        margin: 0;
        padding: 0;
        font-family: "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;

    }
    div {
        width: 600px;
        margin: 5em auto;
        padding: 50px;
        background-color: #fff;
        border-radius: 1em;
    }
    a:link, a:visited {
        color: #38488f;
        text-decoration: none;
    }
    @media (max-width: 700px) {
        body {
            background-color: #fff;
        }
        div {
            width: auto;
            margin: 0 auto;
            border-radius: 0;
            padding: 1em;
        }
    }
    </style>
</head>

<body>
<div>
    <h1>Example Domain</h1>
    <p>This domain is established to be used for illustrative examples in documents. You may use this
