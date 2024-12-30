# Srun🚀

兰州大学校园网命令行认证工具🎓

### Features✨

- 支持Windows💻, Linux🐧
- 使用C++编写💻
- 使用[xmake](https://github.com/xmake-io/xmake)管理和构建🔧

### Usage📜
#### 设置用户名密码🔑
```
lzunc config account "xxx" # 在用户名后添加@study以使用eLearning 
lzunc config password "xxx"
```
自动保存在`config.yaml`

#### 登入
```
lzunc login <IP>
```
可以填入其他接入校园网设备的ip以认证（方便嵌入式设备的连接）

#### 登出
```
lzunc logout
```

#### 查看在线信息📡
```
lzunc status
```

#### 查看流量使用情况📡
```
lzunc info
```
