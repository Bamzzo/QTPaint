# Qt 绘图软件项目总结

这是一个基于 Qt 6 开发的绘图软件，实现了多种绘图工具、历史记录管理、颜色选择、图像保存与加载等功能。项目采用 C++ 面向对象编程思想，所有 UI 组件均通过代码实现。

## 项目亮点

- 🎨 **多种绘图工具**：支持自由绘制、直线、矩形、椭圆、箭头、五角星、菱形、心形、橡皮擦
- ⏪ **历史记录管理**：支持 50 步的撤销/重做功能
- 📂 **文件操作**：支持保存为 PNG/JPEG/BMP 格式，可加载已有图像继续编辑
- 🖱️ **图元编组**：支持选择并移动多个图形
- 🧩 **面向对象设计**：合理运用封装、继承、多态等 OOP 特性
- 📱 **响应式界面**：支持图像缩放和平移操作

## 技术栈

- **开发框架**：Qt 6.9.0
- **编程语言**：C++17
- **编译器**：MinGW 64-bit
- **构建系统**：qmake
- **开发环境**：Qt Creator 16.0.2

## 系统架构

```
表示层 (MainWindow)
├── 工具栏管理
├── 状态栏显示
└── 用户输入处理

业务逻辑层 (PaintArea)
├── 绘图状态管理
├── 命令执行与撤销
└── 坐标系统转换

数据层 (Shape类层次结构)
├── 图形数据存储
├── 绘制算法实现
└── 边界计算
```

## 核心功能实现示例

### 1. 多态图形工厂
```cpp
switch(currentShapeType) {
case Freehand:
    currentShape = new PathShape(...);
    break;
case Line:
    currentShape = new LineShape(...);
    break;
// ...其他图形类型
}
```

### 2. 撤销/重做系统
```cpp
// 保存状态
void PaintArea::saveState() {
    QImage stateImage = ...;
    undoStack.push(stateImage);
    if (undoStack.size() > 50) undoStack.removeFirst();
    redoStack.clear();
}

// 撤销操作
void PaintArea::undo() {
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());
        // 恢复状态...
    }
}
```

### 3. 复杂图形算法（五角星）
```cpp
void StarShape::draw(QPainter& painter) const {
    // 计算五角星顶点
    for (int i = 0; i < 5; ++i) {
        qreal angle = 2 * M_PI * i / 5 - M_PI/2;
        QPointF outerPoint = center + QPointF(radius * cos(angle), ...);
        // 绘制路径...
    }
}
```

## 运行效果

| 版本 | 界面特点 |
|------|----------|
| 1.1 | 基础控件布局 |
| 1.2 | 优化视觉效果 |
| 1.3 | 添加图标和操作指引 |

<img width="519" alt="e9e8cfb0959db3f29280c34f2efad59" src="https://github.com/user-attachments/assets/a46cb03d-20c9-4d8c-93df-c5279192d741" />


## 项目结构

```
PaintProject/
├── main.cpp                # 程序入口
├── mainwindow.h/cpp        # 主窗口实现
├── paintarea.h/cpp         # 绘图区域实现
├── shape.h                 # 图形基类
├── shapes.h/cpp            # 具体图形实现
└── PaintProject.pro        # 项目配置文件
```

## 编译运行

1. 安装 Qt 6.9.0+ 和 MinGW 编译器
2. 打开 `PaintProject.pro` 文件
3. 构建并运行项目

## 未来改进方向

1. **性能优化**：优化复杂图形的绘制算法
2. **功能增强**：添加文本输入、渐变填充等功能
3. **界面美化**：改进UI设计和交互体验
4. **跨平台支持**：适配Linux和macOS系统
5. **稳定性提升**：修复边界条件下的崩溃问题

## 项目收获

通过本项目，深入掌握了：
- Qt框架的核心组件和绘图机制
- C++面向对象设计原则的实际应用
- 图形绘制算法的实现原理
- 撤销/重做等高级功能的实现方式
- 软件调试和性能优化技巧

> 项目已托管在 GitHub，欢迎提出改进建议！
