基于QT6.9.0的画图软件设计与实现实验报告
1. 题目与内容
题目：基于QT6.9.0的画图软件设计与实现
内容：本实验使用C++语言和QT6.9.0框架开发了一个功能完善的画图软件，实现了多种绘图工具、历史记录管理、图像保存与加载等功能，充分体现了面向对象编程思想。

2. 运行环境
硬件环境：

处理器：Intel Core i5及以上
内存：8GB及以上
显示器：分辨率1920×1080及以上
软件环境：

操作系统：Windows 10/11
开发环境：Qt Creator 16.0.2
编译器：MinGW 64-bit
QT版本：6.9.0
3. 课题分析
3.1 需求分析
本画图软件需要满足以下功能需求：

基本绘图功能：自由绘制、直线、矩形、椭圆等基本图形
高级绘图功能：箭头、五角星、菱形、心形等特殊图形
颜色选择与画笔粗细调节
橡皮擦功能
历史记录管理（撤销/重做）
图像保存与加载
图元编组移动功能
3.2 功能结构分析
软件功能模块划分如下：

用户界面模块：负责工具栏、菜单等界面元素的创建和管理
绘图核心模块：负责所有绘图功能的实现
文件管理模块：负责图像的保存和加载
历史记录模块：负责撤销/重做功能的实现
4. 主要模块功能与流程图
4.1 类图设计
contains

1
1
MainWindow

+PaintArea* paintArea

+QColor currentColor

+QPushButton* colorBtn

+QSpinBox* sizeSpinBox

+QComboBox* shapeComboBox

+QAction* undoAction

+QAction* redoAction

+createToolBar()

+changeColor()

+changeBrushSize(int)

+changeShape(int)

+saveImage()

+openImage()

+undo()

+redo()

PaintArea

+enum DrawShape

+QImage image

+QImage tempImage

+QPoint lastPoint

+QPoint firstPoint

+bool drawing

+DrawShape currentShape

+QColor penColor

+int penWidth

+QStack undoStack

+QStack redoStack

+setPenColor(QColor)

+setPenWidth(int)

+setDrawShape(DrawShape)

+saveImage(QString)

+loadImage(QString)

+undo()

+redo()

+paintEvent(QPaintEvent*)

+mousePressEvent(QMouseEvent*)

+mouseMoveEvent(QMouseEvent*)

+mouseReleaseEvent(QMouseEvent*)

+resizeEvent(QResizeEvent*)

+drawShapeToImage(QPoint)

+drawStar(QPainter&, QRect)

+drawDiamond(QPainter&, QRect)

+drawHeart(QPainter&, QRect)

+saveState()

4.2 核心类说明
MainWindow类：

继承自QMainWindow，是应用程序的主窗口
负责创建和管理工具栏、菜单等界面元素
处理用户交互事件，如颜色选择、形状选择等
包含一个PaintArea实例作为中央部件
PaintArea类：

继承自QWidget，是实际的绘图区域
实现了所有绘图功能，包括自由绘制、各种图形绘制等
管理撤销/重做栈
处理鼠标事件，实现绘图交互
实现图像的保存和加载功能
4.3 关键功能流程图
绘图流程：

PaintArea
MainWindow
User
PaintArea
MainWindow
User
选择绘图工具
setDrawShape(shape)
鼠标按下
记录起点，创建临时图像
鼠标移动
在临时图像上绘制图形
更新显示
鼠标释放
将临时图像合并到主图像
保存状态到撤销栈
撤销/重做流程：

PaintArea
MainWindow
User
PaintArea
MainWindow
User
点击撤销按钮
undo()
从撤销栈弹出当前状态
将当前状态压入重做栈
恢复上一状态
更新显示
5. 调试、测试、运行记录及分析
5.1 调试过程
绘图闪烁问题：

现象：绘制图形时出现闪烁
原因：直接在主图像上绘制，没有使用双缓冲
解决：引入tempImage作为临时绘图缓冲区，只在鼠标释放时才合并到主图像
撤销/重做功能异常：

现象：多次撤销后程序崩溃
原因：撤销栈未做边界检查
解决：添加栈空检查，并限制最大历史记录数量(50)
图像缩放问题：

现象：加载大图像时显示不全
原因：未正确处理图像缩放和偏移
解决：实现物理坐标与逻辑坐标转换系统
5.2 测试记录
测试项	测试结果	备注
自由绘制	通过	线条平滑，颜色和粗细可调
基本图形绘制	通过	矩形、椭圆等图形绘制正常
特殊图形绘制	通过	五角星、心形等绘制正常
颜色选择	通过	颜色对话框正常弹出，颜色应用正确
画笔粗细调整	通过	实时生效，范围1-100
橡皮擦功能	通过	能正确擦除各种图形
撤销/重做	通过	最多支持50步历史记录
图像保存	通过	保存为PNG格式，质量无损
图像加载	通过	能正确加载并继续编辑
图元编组移动	通过	能正确选择并移动一组图元
5.3 性能分析
内存占用：在绘制复杂图形时，内存占用会有所增加，主要来自图像数据和历史记录栈
响应速度：基本绘图操作响应迅速，加载大图像时会有短暂延迟
历史记录：限制为50步，平衡了功能需求和内存消耗
6. 总结
6.1 优点
面向对象设计：

良好的类划分，职责单一
使用继承和多态等面向对象特性
模块化设计，便于扩展
功能完善：

实现了多种绘图工具
完整的撤销/重做系统
支持图像保存和加载
用户体验：

直观的工具栏设计
实时预览绘制效果
支持大图像浏览和编辑
6.2 不足
性能优化：

处理超大图像时性能有待提升
历史记录占用内存较多
功能扩展：

缺少文本输入功能
缺少更高级的图形编辑功能
用户体验：

缺少自定义快捷键
界面美化程度不足
6.3 收获与体会
通过本次实验，深入理解了：

QT框架的绘图机制和事件处理系统
面向对象设计原则在实际项目中的应用
图形编辑软件的基本架构和实现方法
用户体验设计的重要性
未来可以进一步扩展的功能包括：图层支持、滤镜效果、更多图形工具等，这将使软件功能更加完善。
