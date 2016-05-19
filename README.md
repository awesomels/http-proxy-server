# http-proxy-server
####一个http代理服务器
目前进度在接受单次的来自浏览器的http请求
<br/>
已经能够解析出Host，并转换成ip地址（域名转IP）
<br/>
目前的想法是epoll+线程池，还需要一个映射的数据结构提供查询
