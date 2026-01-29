# Test Device Driver - 使用文档

## 📁 测试设备驱动说明

这是一个演示 VFS 使用的简单字符设备驱动。

## 🎯 功能特性

- **设备名称**: `/testdev`
- **缓冲区大小**: 1024 字节
- **支持操作**: open, close, read, write, seek

## 📂 文件结构

```
drivers/testdev/
├── testdev.h       # 头文件
└── testdev.c       # 实现文件
```

## 🔧 实现细节

### 核心数据结构

```c
static char testdev_buffer[1024];  // 设备缓冲区
static uint32_t testdev_len = 0;   // 当前数据长度
```

### 操作函数

| 函数 | 功能 | 返回值 |
|------|------|--------|
| `testdev_open()` | 打开设备 | 0 (成功) |
| `testdev_close()` | 关闭设备 | 0 (成功) |
| `testdev_read()` | 读取数据 | 读取字节数 |
| `testdev_write()` | 写入数据 | 写入字节数 |
| `testdev_seek()` | 移动文件指针 | 0 (成功) / -1 (失败) |

## 💡 使用方法

### 方法 1: Shell 命令 (最简单)

```bash
> testdev
```

这会自动执行完整的测试流程：
1. 打开 `/testdev`
2. 写入测试数据 "Hello from VFS test!"
3. Seek 到开头
4. 读取并显示数据
5. 关闭设备

### 方法 2: 通过 VFS API (C 代码)

```c
#include "fs/vfs.h"

void test_device(void) {
    // 1. 打开设备
    file_t *file = vfs_open("/testdev", 0);
    if (file == NULL) {
        printf("Failed to open device\n");
        return;
    }
    
    // 2. 写入数据
    const char *data = "Hello, World!";
    int written = vfs_write(file, data, 13);
    printf("Wrote %d bytes\n", written);
    
    // 3. 重置文件指针
    file->inode->ops->seek(file, 0);
    
    // 4. 读取数据
    char buffer[64];
    int read_bytes = vfs_read(file, buffer, sizeof(buffer));
    buffer[read_bytes] = '\0';
    printf("Read: %s\n", buffer);
    
    // 5. 关闭设备
    vfs_close(file);
}
```

### 方法 3: 通过系统调用 (用户程序)

```c
// 用户态程序
int main() {
    // 打开设备
    int fd = syscall(SYS_OPEN, "/testdev", 0);
    
    // 写入
    const char *msg = "User data";
    syscall(SYS_WRITE, fd, msg, 9);
    
    // 读取
    char buf[32];
    int n = syscall(SYS_READ, fd, buf, sizeof(buf));
    
    // 关闭
    syscall(SYS_CLOSE, fd);
    
    return 0;
}
```

## 📊 示例输出

```
> testdev
[TEST] Testing /testdev device
[TESTDEV] Device opened
[TESTDEV] Wrote 20 bytes (offset now 20, total 20)
[TEST] Wrote 20 bytes
[TESTDEV] Seek to offset 0
[TESTDEV] Read 20 bytes (offset now 20)
[TEST] Read 20 bytes: Hello from VFS test!
[TESTDEV] Device closed
[TEST] Test completed
```

## 🔍 工作流程

```
应用层 (Shell)
    ↓ testdev 命令
VFS 层
    ↓ vfs_open("/testdev")
设备查找
    ↓ find_device("testdev")
创建 file_t
    ↓ 绑定 testdev_ops
调用驱动
    ↓ testdev_open() / testdev_write() / testdev_read()
操作缓冲区
    ↓ testdev_buffer[1024]
返回结果
    ↓
应用层显示
```

## 🎓 学习要点

### 1. 设备注册
```c
// 在 kernel_main() 中
testdev_init();          // 初始化设备
testdev_register();      // 注册到 VFS
```

### 2. 操作函数表
```c
static file_ops_t testdev_ops = {
    .open = testdev_open,
    .close = testdev_close,
    .read = testdev_read,
    .write = testdev_write,
    .seek = testdev_seek
};
```

### 3. VFS 调用链
```
vfs_open()
  → find_device()
  → create file_t + inode_t
  → inode->ops = &testdev_ops
  → testdev_open()
  
vfs_read()
  → file->inode->ops->read()
  → testdev_read()
```

## 🚀 扩展建议

### 添加新设备驱动

1. **复制模板**
   ```bash
   cp -r drivers/testdev drivers/mydev
   ```

2. **修改实现**
   - 实现自己的 open/close/read/write
   - 根据设备特性调整缓冲区

3. **注册设备**
   ```c
   mydev_init();
   vfs_register_device("mydev", &mydev_ops);
   ```

### 实际设备示例

#### 内存磁盘 (RAM Disk)
```c
static uint8_t ramdisk[1024*1024];  // 1MB
// 实现块设备接口
```

#### 随机数生成器
```c
int random_read(file_t *file, void *buf, size_t count) {
    uint8_t *cbuf = (uint8_t*)buf;
    for (size_t i = 0; i < count; i++) {
        cbuf[i] = get_random_byte();
    }
    return count;
}
```

#### 串口设备
```c
int serial_write(file_t *file, const void *buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uart_putc(((char*)buf)[i]);
    }
    return count;
}
```

## ✅ 测试检查清单

- [ ] 编译无错误
- [ ] Shell 中 `testdev` 命令可用
- [ ] 能正确写入数据
- [ ] Seek 操作正常
- [ ] 能读取写入的数据
- [ ] 数据内容正确
- [ ] 设备正确关闭

## 🐛 常见问题

**Q: 设备未注册成功？**
A: 检查 `vfs_init()` 是否在 `testdev_register()` 之前调用

**Q: 读写操作失败？**
A: 检查文件指针偏移量，是否需要 seek

**Q: 缓冲区溢出？**
A: testdev 限制 1024 字节，超出会返回错误

---

🎉 **恭喜！** 你已经学会了如何编写 VFS 设备驱动！
