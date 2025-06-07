#include "paintarea.h"
#include <QPainterPath>
#include <QDebug>
#include <QResizeEvent>
#include <cmath>
#include <QFileDialog>
#include "shapes.h"

// 构造函数，初始化绘图区域
PaintArea::PaintArea(QWidget *parent) : QWidget(parent)
{
    // 设置大小策略为可扩展
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 设置静态内容属性，优化绘制性能
    setAttribute(Qt::WA_StaticContents);
    // 启用鼠标跟踪
    setMouseTracking(true);

    // 初始化成员变量
    isSelecting = false;          // 是否正在选择区域
    selectionRect = QRect();      // 选择区域矩形
    drawing = false;              // 是否正在绘制
    currentShapeType = Freehand;  // 默认绘制类型为自由绘制
    // 创建800x600的透明背景图像
    image = QImage(800, 600, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);        // 填充白色背景
    tempImage = image;            // 临时图像用于预览

    // 默认画笔设置
    penColor = Qt::black;         // 黑色画笔
    penWidth = 3;                 // 3像素宽度
    undoStack.push(image);        // 初始状态压入撤销栈
}

// 设置画笔颜色
void PaintArea::setPenColor(const QColor &color)
{
    penColor = color;
}

// 设置画笔宽度
void PaintArea::setPenWidth(int width)
{
    penWidth = width;
}

// 设置当前绘制形状类型
void PaintArea::setDrawShape(DrawShape shape)
{
    currentShapeType = shape;
}

// 更新缩放比例和偏移量
void PaintArea::updateScaleAndOffset()
{
    QSize widgetSize = size();  // 获取当前控件大小

    if (originalImage.isNull()) {
        // 如果没有原始图像，使用1:1比例
        scaleFactor = 1.0;
        offset = QPoint(0, 0);
        origImageSize = widgetSize;
    } else {
        // 计算适合控件大小的缩放比例
        origImageSize = originalImage.size();
        scaleFactor = qMin(static_cast<double>(widgetSize.width()) / origImageSize.width(),
                           static_cast<double>(widgetSize.height()) / origImageSize.height());
        // 计算居中偏移量
        offset = QPoint((widgetSize.width() - origImageSize.width() * scaleFactor) / 2,
                        (widgetSize.height() - origImageSize.height() * scaleFactor) / 2);
    }
}

// 物理坐标(窗口坐标)转换为逻辑坐标(图像坐标)
QPoint PaintArea::physicalToLogical(const QPoint &physicalPoint) const
{
    QPoint adjustedPoint = physicalPoint;

    // 如果有原始图像，确保坐标在图像区域内
    if (!originalImage.isNull()) {
        QRect contentRect(offset, origImageSize * scaleFactor);
        adjustedPoint.setX(qBound(contentRect.left(), physicalPoint.x(), contentRect.right()));
        adjustedPoint.setY(qBound(contentRect.top(), physicalPoint.y(), contentRect.bottom()));
    }

    // 如果没有原始图像，确保坐标在控件范围内
    if (originalImage.isNull()) {
        return QPoint(
            qBound(0, physicalPoint.x(), width()-1),
            qBound(0, physicalPoint.y(), height()-1));
    }

    // 计算逻辑坐标
    int x = qBound(0.0, (physicalPoint.x() - offset.x()) / scaleFactor, origImageSize.width() - 1.0);
    int y = qBound(0.0, (physicalPoint.y() - offset.y()) / scaleFactor, origImageSize.height() - 1.0);
    return QPoint(x, y);
}

// 逻辑坐标(图像坐标)转换为物理坐标(窗口坐标)
QPoint PaintArea::logicalToPhysical(const QPoint &logicalPoint) const
{
    if (originalImage.isNull()) {
        return logicalPoint;
    }
    return QPoint(
        logicalPoint.x() * scaleFactor + offset.x(),
        logicalPoint.y() * scaleFactor + offset.y()
        );
}

// 逻辑矩形(图像坐标)转换为物理矩形(窗口坐标)
QRect PaintArea::logicalToPhysical(const QRect &logicalRect) const
{
    return QRect(logicalToPhysical(logicalRect.topLeft()),
                 QSize(logicalRect.width() * scaleFactor,
                       logicalRect.height() * scaleFactor));
}

// 控件大小改变事件处理
void PaintArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaleAndOffset();  // 更新缩放和偏移

    // 如果有原始图像
    if (!originalImage.isNull()) {
        if (image.size() != originalImage.size()) {
            // 创建新图像并保持原有内容
            QImage newImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
            newImage.fill(Qt::transparent);
            QPainter painter(&newImage);
            painter.drawImage(0, 0, image);
            image = newImage;
        }
    } else {
        // 如果没有原始图像，调整图像大小
        if (image.size() != event->size()) {
            QImage newImage(event->size(), QImage::Format_ARGB32_Premultiplied);
            newImage.fill(Qt::transparent);
            QPainter painter(&newImage);
            painter.drawImage(0, 0, image);
            image = newImage;
        }
    }

    tempImage = image;  // 重置临时图像
    update();           // 触发重绘
}

// 保存图像到文件
void PaintArea::saveImage(const QString &fileName)
{
    // 创建最终图像：如果有原始图像则复制原始图像，否则使用当前图像
    QImage finalImage = originalImage.isNull() ? image : originalImage.copy();
    QPainter painter(&finalImage);
    painter.drawImage(0, 0, image);  // 将绘制内容合并到最终图像
    finalImage.save(fileName, "PNG"); // 保存为PNG格式
}

// 加载图像文件
void PaintArea::loadImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName)) return;  // 加载失败则返回

    // 保存当前状态到撤销栈
    saveState();

    // 转换图像格式并保存为原始图像
    originalImage = loadedImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    image = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);  // 透明背景

    // 保存仅包含图片的状态到撤销栈
    QImage initialState = originalImage.copy();
    undoStack.push(initialState);
    redoStack.clear();  // 清空重做栈

    updateScaleAndOffset();  // 更新缩放和偏移
    update();               // 触发重绘
}

// 绘制事件处理
void PaintArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);  // 避免未使用参数警告

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 启用平滑变换
    painter.fillRect(rect(), Qt::white);  // 填充白色背景

    // 如果有原始图像，绘制原始图像
    if (!originalImage.isNull()) {
        QRect drawRect(offset.x(), offset.y(),
                       origImageSize.width() * scaleFactor,
                       origImageSize.height() * scaleFactor);
        painter.drawImage(drawRect, originalImage);
    }

    // 绘制当前图像内容
    QRect contentRect = !originalImage.isNull() ?
                            QRect(offset, origImageSize * scaleFactor) :
                            rect();
    painter.drawImage(contentRect, image, image.rect());

    // 如果正在绘制，绘制临时图像(预览)
    if (drawing) {
        painter.drawImage(contentRect, tempImage, tempImage.rect());
    }

    // 如果正在选择区域，绘制选择框
    if (isSelecting) {
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));  // 蓝色虚线
        painter.drawRect(QRect(
            logicalToPhysical(selectionRect.topLeft()),
            logicalToPhysical(selectionRect.bottomRight()))
                         );
    }
}

// 鼠标按下事件处理
void PaintArea::mousePressEvent(QMouseEvent *event)
{
    // 如果是区域选择模式
    if (currentShapeType == GroupSelect) {
        selectionStart = physicalToLogical(event->pos());  // 记录选择起点
        isSelecting = true;
        return;
    }

    // 左键按下开始绘制
    if (event->button() == Qt::LeftButton) {
        QPoint logicalPoint = physicalToLogical(event->pos());  // 转换为逻辑坐标
        drawing = true;
        tempImage = image.copy();  // 复制当前图像到临时图像

        // 根据当前形状类型创建对应的Shape对象
        switch(currentShapeType) {
        case Freehand:  // 自由绘制
            currentShape = new PathShape(logicalPoint, penColor, penWidth, false);
            break;
        case Line:      // 直线
            currentShape = new LineShape(logicalPoint, penColor, penWidth);
            break;
        case Rectangle: // 矩形
            currentShape = new RectangleShape(logicalPoint, penColor, penWidth);
            break;
        case Ellipse:   // 椭圆
            currentShape = new EllipseShape(logicalPoint, penColor, penWidth);
            break;
        case Arrow:     // 箭头
            currentShape = new ArrowShape(logicalPoint, penColor, penWidth);
            break;
        case Star:      // 星形
            currentShape = new StarShape(logicalPoint, penColor, penWidth);
            break;
        case Diamond:   // 菱形
            currentShape = new DiamondShape(logicalPoint, penColor, penWidth);
            break;
        case Heart:     // 心形
            currentShape = new HeartShape(logicalPoint, penColor, penWidth);
            break;
        case Eraser:    // 橡皮擦
            currentShape = new PathShape(logicalPoint, Qt::white, penWidth, true);
            break;
        case GroupSelect: // 已在上面处理
            break;
        }
    }
}

// 鼠标移动事件处理
void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    // 发射光标位置变化信号
    QPoint currentLogicalPos = physicalToLogical(event->pos());
    emit cursorPositionChanged(currentLogicalPos);

    // 如果是区域选择模式
    if (isSelecting) {
        selectionEnd = currentLogicalPos;
        selectionRect = QRect(selectionStart, selectionEnd).normalized();  // 标准化矩形
        tempImage = image.copy();
        // 在选择区域绘制虚线框
        QPainter painter(&tempImage);
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawRect(selectionRect);
        update();  // 触发重绘
        return;
    }

    // 如果正在绘制且有当前形状
    if ((event->buttons() & Qt::LeftButton) && drawing && currentShape) {
        currentShape->update(currentLogicalPos);  // 更新形状

        // 实时绘制到临时图像
        tempImage = image.copy();
        QPainter painter(&tempImage);
        currentShape->draw(painter);  // 绘制当前形状

        update();  // 触发重绘
    }
}

// 鼠标释放事件处理
void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果是区域选择模式
    if (isSelecting) {
        isSelecting = false;
        if (!selectionRect.isNull()) {
            // 复制选择区域并移动
            QImage newImage = image.copy();
            QPainter painter(&newImage);
            QPoint logicalEnd = physicalToLogical(event->pos());
            QPoint offset = logicalEnd - selectionStart;

            QImage selectedArea = image.copy(selectionRect);
            painter.fillRect(selectionRect, Qt::white);  // 清除原位置
            painter.drawImage(selectionRect.translated(offset), selectedArea, selectionRect);

            image = newImage;
            saveState();  // 保存状态
            update();     // 触发重绘
        }
        return;
    }

    // 如果是左键释放且正在绘制
    if (event->button() == Qt::LeftButton && drawing && currentShape) {
        QPainter painter(&image);
        currentShape->draw(painter);  // 将形状绘制到主图像

        delete currentShape;  // 释放形状对象
        currentShape = nullptr;

        saveState();    // 保存状态
        drawing = false; // 结束绘制
        update();       // 触发重绘
    }
}

// 清除选择区域
void PaintArea::clearSelection()
{
    isSelecting = false;
    selectionRect = QRect();
    update();  // 触发重绘
}

// 撤销操作
void PaintArea::undo()
{
    if (undoStack.size() > 1) {
        redoStack.push(undoStack.pop());  // 从撤销栈弹出并压入重做栈
        QImage stateImage = undoStack.top();  // 获取上一个状态

        // 恢复状态
        originalImage = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        originalImage.fill(Qt::transparent);
        QPainter painter(&originalImage);
        painter.drawImage(0, 0, stateImage);

        image = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        // 触发大小调整事件
        QResizeEvent fakeEvent(size(), size());
        resizeEvent(&fakeEvent);
        update();  // 触发重绘
    }
}

// 重做操作
void PaintArea::redo()
{
    if (!redoStack.isEmpty()) {
        undoStack.push(redoStack.pop());  // 从重做栈弹出并压入撤销栈
        QImage stateImage = undoStack.top();  // 获取状态

        // 恢复状态
        originalImage = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        originalImage.fill(Qt::transparent);
        QPainter painter(&originalImage);
        painter.drawImage(0, 0, stateImage);

        image = QImage(stateImage.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        // 触发大小调整事件
        QResizeEvent fakeEvent(size(), size());
        resizeEvent(&fakeEvent);
        update();  // 触发重绘
    }
}

// 保存当前状态到撤销栈
void PaintArea::saveState()
{
    // 创建状态图像：如果有原始图像则使用原始图像，否则使用当前图像
    QImage stateImage = originalImage.isNull() ?
                            image.copy() :
                            originalImage.copy();

    // 将当前绘制内容合并到状态图像
    QPainter painter(&stateImage);
    painter.drawImage(0, 0, image);

    // 如果状态有变化，保存到撤销栈
    if (undoStack.isEmpty() || undoStack.top() != stateImage) {
        undoStack.push(stateImage);
        if (undoStack.size() > 50) undoStack.removeFirst();  // 限制栈大小
        redoStack.clear();  // 清空重做栈
    }
}
