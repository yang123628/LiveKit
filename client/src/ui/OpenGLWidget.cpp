#include "ui/OpenGLWidget.h"

OpenGLWidget::OpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_program(nullptr)
    , m_texture(nullptr)
    , m_textureCreated(false)
{
    setMinimumSize(320, 240);
}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    if (m_program) {
        delete m_program;
        m_program = nullptr;
    }
    doneCurrent();
}

void OpenGLWidget::updateFrame(const QImage& image) {
    QMutexLocker locker(&m_frameMutex);
    m_frame = image.copy();
    update();
}

void OpenGLWidget::clearFrame() {
    QMutexLocker locker(&m_frameMutex);
    m_frame = QImage();
    update();
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_program = new QOpenGLShaderProgram(this);

    const char* vertexSrc =
        "attribute vec2 a_position;\n"
        "attribute vec2 a_texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "    v_texCoord = a_texCoord;\n"
        "}\n";

    const char* fragmentSrc =
        "uniform sampler2D u_texture;\n"
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(u_texture, v_texCoord);\n"
        "}\n";

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSrc);
    m_program->link();
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    QImage currentFrame;
    {
        QMutexLocker locker(&m_frameMutex);
        currentFrame = m_frame;
    }

    if (currentFrame.isNull()) return;

    QImage glImage = currentFrame.convertToFormat(QImage::Format_RGB888);

    if (m_texture) {
        delete m_texture;
    }
    m_texture = new QOpenGLTexture(glImage.mirrored(false, true));
    m_texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);

    m_program->bind();

    GLfloat vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    GLfloat texCoords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    int posLoc = m_program->attributeLocation("a_position");
    int texLoc = m_program->attributeLocation("a_texCoord");

    m_texture->bind();

    m_program->enableAttributeArray(posLoc);
    m_program->setAttributeArray(posLoc, vertices, 2);
    m_program->enableAttributeArray(texLoc);
    m_program->setAttributeArray(texLoc, texCoords, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->disableAttributeArray(posLoc);
    m_program->disableAttributeArray(texLoc);

    m_texture->release();
    m_program->release();
}

void OpenGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}
