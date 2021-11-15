#ifndef WIDGETOPENGLDRAW_H
#define WIDGETOPENGLDRAW_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class WidgetOpenGLDraw : public QOpenGLWidget{
public:
	WidgetOpenGLDraw(QWidget* parent);

	~WidgetOpenGLDraw();
    void NarediNekajRotacija();
    void NarediNekajDodajZelenoBarvo();

private:
	void PrevediSencilnike();
	void printProgramInfoLog(GLuint obj);
	void printShaderInfoLog(GLuint obj);



    QOpenGLFunctions_3_3_Core* gl;
	unsigned int id_sencilni_program;

	unsigned int id_buffer_trikotnik;
    unsigned int id_plane;

	GLuint id_VAO_trikotnik;

    float mosX = 0.0f;
    float mosY = 0.0f;
    float rotX=0.0f;
    float rotY=0.0f;
    float rotZ=0.0f;
    float dodajZelenoBarvo=0;
    float positionX = 0.0f;

    float translateX = 0.0f;
    float translateY = 0.0f;
    float translateZ = 0.0f;

    //float deltaTime = 0.0f;
    //float lastFrame = 0.0f;
protected:

	void paintGL() override;
	void initializeGL() override;
	void resizeGL(int w, int h) override;
    virtual void keyPressEvent(QKeyEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

signals:

public slots:

};

#endif // WIDGETOPENGLDRAW_H
