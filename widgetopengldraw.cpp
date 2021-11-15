#include "widgetopengldraw.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QOpenGLFunctions_3_3_Core>
#include <QtGlobal>
#include <chrono>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <ctime>
#if QT_VERSION >= 0x060000
#include <QOpenGLVersionFunctionsFactory>
#endif

#include <QApplication>

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
std::chrono::time_point lastFrame = Time::now();
std::chrono::time_point deltaTime = Time::now();

bool fx = false;
bool fy = false;
bool fz = false;

bool isOrtho = false;


glm::vec3 position = glm::vec3( 0, 0, 5 );


WidgetOpenGLDraw::WidgetOpenGLDraw(QWidget *parent) : QOpenGLWidget(parent) {
}

WidgetOpenGLDraw::~WidgetOpenGLDraw() {
    //počisti stanje
    gl->glDeleteVertexArrays(1,&id_VAO_trikotnik);
    gl->glDeleteProgram(id_sencilni_program);
}


void WidgetOpenGLDraw::printProgramInfoLog(GLuint obj) {
    int infologLength = 0;
    gl->glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        std::unique_ptr<char[]> infoLog(new char[infologLength]);
        int charsWritten = 0;
        gl->glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog.get());
        std::cerr << infoLog.get() << "\n";
    }
}

void WidgetOpenGLDraw::printShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    gl->glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        std::unique_ptr<char[]> infoLog(new char[infologLength]);
        int charsWritten = 0;
        gl->glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog.get());
        std::cerr << infoLog.get() << "\n";
    }
}

void WidgetOpenGLDraw::PrevediSencilnike() {
    id_sencilni_program = gl->glCreateProgram();

    {  // vhod v senčilnik oglišč je in_Pos, izhod pa gl_Position (rezervirana beseda)
        GLuint vs = gl->glCreateShader(GL_VERTEX_SHADER);
        std::string vss;      // priporočamo hrambo spodnjega niza v datoteki (vsencilnik.vert), potem dobite barvanje sintakse in autocomplete
        vss += " #version 330	                                 \n";
        vss += " layout(location=0) in vec3 in_Pos;	             \n";
        vss += " uniform mat4 PVM;						         \n";
        vss += " out vec3 gPos;						             \n";
        vss += " void main(){						             \n";
        vss += "  gl_Position=PVM*vec4(in_Pos.xyz,1);          \n"; // Za 2. vajo potrebujete projekcijsko matriko (glede na predavanja)
        //vss+="      gl_Position=vec4(in_Pos.xyz,1);             \n"; // v cevovod lahko dalje pošljemo kar tisto, kar dobimo kot vhod
        vss+="      gPos=in_Pos;                                 \n";
        vss += " }                                               \n ";
        std::cout << vss;
        const char *vssc = vss.c_str();
        gl->glShaderSource(vs, 1, &vssc, nullptr);
        gl->glCompileShader(vs);
        printShaderInfoLog(vs);
        gl->glAttachShader(id_sencilni_program, vs);
    }

    {  // out_Color je barva, ki bo prišla do zaslona
        GLuint fs = gl->glCreateShader(GL_FRAGMENT_SHADER);
        std::string fss;  // priporočamo hrambo spodnjega niza v datoteki (fsencilnik.frag), potem dobite barvanje sintakse in autocomplete
        fss += " #version 330					                \n";
        fss += " out vec4 out_Color;	                        \n";
        fss += " in vec3 gPos;						            \n";
        fss += " uniform vec4 DodajBarvo;						         \n";
        fss += " void main(){			                        \n";
        fss += "  out_Color=vec4(gPos.x,gPos.y ,0,1)+DodajBarvo;           \n";
        fss += " }						                        \n";
        std::cout << fss;
        const char *fssc = fss.c_str();
        gl->glShaderSource(fs, 1, &fssc, nullptr);
        gl->glCompileShader(fs);
        printShaderInfoLog(fs);
        gl->glAttachShader(id_sencilni_program, fs);
    }

    gl->glLinkProgram(id_sencilni_program);
    printProgramInfoLog(id_sencilni_program);
}

void WidgetOpenGLDraw::initializeGL() {


    //Focus
    setFocusPolicy(Qt::StrongFocus);
    // naložimo funkcije za OpenGL
    std::cout << "OpenGL context version: "<< context()->format().majorVersion() <<"." <<context()->format().minorVersion()<<std::endl;

    //  ali uporabljate najnovejši QT6 ali 5
#if QT_VERSION >= 0x060000
    gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
#else
    gl=context()->versionFunctions<QOpenGLFunctions_3_3_Core>();
#endif


    if (!gl) {
        std::cerr << "Could not obtain required OpenGL context version";
        QApplication::exit(1);
    }


    std::cout << gl->glGetString(GL_VENDOR) << std::endl;
    std::cout << gl->glGetString(GL_VERSION) << std::endl;
    std::cout << gl->glGetString(GL_RENDERER) << std::endl;
    //WIREFRAME
    //gl->glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


    PrevediSencilnike();
    // gl->glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    //  gl->glDepthFunc(GL_LESS);

    gl->glEnable(GL_DEPTH_TEST);		//v primeru, da rišemo več prekrivajočih
    // trikotnikov, želimo, da sprednje ne prekrijejo tisti, ki bi morali biti
    // odzadaj
    gl->glDisable(GL_CULL_FACE);		//rišemo obe lici trikotnikov

    // dejansko nosi lastnosti povezane z buffer (npr. stanje od
    // glEnableVertexAttribArray itd)
    // uporabno predvsem za večjo hitrost
    gl->glGenVertexArrays(1, &id_VAO_trikotnik);
    gl->glBindVertexArray(id_VAO_trikotnik);

    // naložimo trikotnik na GPU in določimo podatke
    //viewport -> območje izrisa v OpenGL je med -1 in 1 po obeh oseh
    const glm::vec3 trikotnik[] = {glm::vec3(-0.5, -0.5, -0.5),glm::vec3(-0.5, -0.5, 0.5),glm::vec3(-0.5, 0.5, 0.5),
                                   glm::vec3(0.5, 0.5, -0.5), glm::vec3(-0.5, -0.5, -0.5),glm::vec3(-0.5, 0.5, -0.5),
                                   glm::vec3(0.5, -0.5, 0.5), glm::vec3(-0.5, -0.5, -0.5),glm::vec3(0.5, -0.5, -0.5), //Spodnja stran

                                   glm::vec3(0.5, 0.5, -0.5),glm::vec3(0.5, -0.5, -0.5),glm::vec3(-0.5, -0.5, -0.5), //Spodnja stran(2)
                                   glm::vec3(-0.5, -0.5, -0.5),glm::vec3(-0.5, 0.5, 0.5),glm::vec3(-0.5, 0.5, -0.5),
                                   glm::vec3(0.5, -0.5, 0.5),glm::vec3(-0.5, -0.5, 0.5),glm::vec3(-0.5, -0.5, -0.5),


                                   glm::vec3(-0.5, 0.5, 0.5),glm::vec3(-0.5, -0.5, 0.5),glm::vec3(0.5, -0.5, 0.5),
                                   glm::vec3(0.5, 0.5, 0.5),glm::vec3(0.5, -0.5, -0.5),glm::vec3(0.5, 0.5, -0.5),
                                   glm::vec3(0.5, -0.5, -0.5),glm::vec3(0.5, 0.5, 0.5),glm::vec3(0.5, -0.5, 0.5),

                                   glm::vec3(0.5, 0.5, 0.5),glm::vec3(0.5, 0.5, -0.5),glm::vec3(-0.5, 0.5, -0.5),
                                   glm::vec3(0.5, 0.5, 0.5),glm::vec3(-0.5, 0.5, -0.5),glm::vec3(-0.5, 0.5, 0.5),
                                   glm::vec3(0.5, 0.5, 0.5),glm::vec3(-0.5, 0.5, 0.5),glm::vec3(0.5, -0.5, 0.5),

                              };

    const glm::vec3 plane[] = {glm::vec3(-3.5, -0.5, 3.5), glm::vec3(-3.5, -0.5, -3.5),glm::vec3(3.5, -0.5, -3.5),
                               glm::vec3(-3.5, -0.5, 3.5),glm::vec3(3.5, -0.5, -3.5),glm::vec3(3.5, -0.5, 3.5)};


    gl->glGenBuffers(1, &id_plane);
        gl->glBindBuffer(GL_ARRAY_BUFFER, id_plane);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 6, plane, GL_STATIC_DRAW);
        gl->glEnableVertexAttribArray(0);  // uporabjamo: layout(location=0) in vec3 in_Pos;
        gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);



    gl->glGenBuffers(1, &id_buffer_trikotnik);
    gl->glBindBuffer(GL_ARRAY_BUFFER, id_buffer_trikotnik);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 36, trikotnik, GL_STATIC_DRAW);
    gl->glEnableVertexAttribArray(0);  // uporabjamo: layout(location=0) in vec3 in_Pos;
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    const unsigned int err = gl->glGetError();
    if (err != 0) {
        std::cerr << "OpenGL init napaka: " << err << std::endl;
    }
}

void WidgetOpenGLDraw::resizeGL(int w, int h) {
    gl->glViewport(0, 0, w, h);
}

void WidgetOpenGLDraw::paintGL() {


    auto currentFrame = Time::now();

    fsec fs = currentFrame - lastFrame;
    lastFrame = currentFrame;
    ms d = std::chrono::duration_cast<ms>(fs);


    float deltaTime = fs.count();

    // počisti ozadje in globinski pomnilnik (za testiranje globine)
    gl->glClearColor(0.2f, 0.2f, 0.2f, 1);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    gl->glBindVertexArray(id_VAO_trikotnik);
    gl->glUseProgram(id_sencilni_program);

    // position

    // horizontal angle : toward -Z
    float horizontalAngle = 3.14f;
    // vertical angle : 0, look at the horizon
    float verticalAngle = 0.0f;
    // Initial Field of View
    float initialFoV = 45.0f;

    float speed = 3.0f; // 3 units / second
    float mouseSpeed = 0.005f;

    horizontalAngle += mouseSpeed * deltaTime * float(1109/2 - mosX );
    verticalAngle   += mouseSpeed  * deltaTime*  float( 606/2 - mosY );

    mosX = mosX *0.005f;
    mosY = mosY *0.005f;

//mouse

    glm::vec3 direction(
                cos(mosY) * sin(mosX),
                sin(mosY),
                cos(mosY) * cos(mosX)
                );

    glm::vec3 right = glm::vec3(
                sin(horizontalAngle - 3.14f/2.0f),
                0,
                cos(horizontalAngle - 3.14f/2.0f)
                );

    glm::vec3 up = glm::cross( right, direction );

    glm::mat4 Projection;





    if (isOrtho){
       Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);
    }else{
       Projection = glm::perspective(glm::radians(45.0f), (float) 1109 / (float)606, 0.1f, 100.0f);
    }





    glm::mat4 View = glm::lookAt(
                glm::vec3(4,3,3) + positionX,
                glm::vec3(0,0,0) ,
                glm::vec3(0,1,0)
                );

    /*View = glm::lookAt(
            position,
            position+direction,
            up
            );*/


    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 PVM = Projection * View * Model;
    glm::mat4 PVM2 = PVM;

    gl->glUniformMatrix4fv(gl->glGetUniformLocation(id_sencilni_program, "PVM"), 1, GL_FALSE, glm::value_ptr(PVM));

    glm::mat4 I(1.0);//identiteta
    I = glm::rotate_slow(I, glm::radians(float(rotX)), glm::vec3(0, 0, 1));
    Model = glm::rotate_slow(I, glm::radians(float(rotX)), glm::vec3(0, 0, 1));

    glm::vec3 rotateMatrix = glm::vec3(1.0f,1.0f,1.0f);


    //podamo barvo v obliki RGBA! v senčilniku fragmentov jo prištejemo!
    gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 1,dodajZelenoBarvo,0,0);

    //Poglej bool, da ves na katero os se mora kocka premikat

    if (fx) {

        rotateMatrix = glm::vec3(1.0f, 0.0f,0.0f);

    }else if(fy){
        rotateMatrix = glm::vec3(0.0f, 1.0f,0.0f);
    }
    else if(fz){
        rotateMatrix = glm::vec3(0.0f, 0.0f,1.0f);
    }

    //Zgornja kocka
    glm::mat4 cube2ModelToWorldMatrix = glm::translate(glm::vec3(0.0f + translateX, 1.0f + translateY, 0.0f+ translateZ)) * glm::rotate(glm::radians(rotX), rotateMatrix);
    PVM = PVM*cube2ModelToWorldMatrix;

    gl->glUniformMatrix4fv(gl->glGetUniformLocation(id_sencilni_program, "PVM"), 1, GL_FALSE, glm::value_ptr(PVM));
    gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 1,dodajZelenoBarvo,0,0);
    gl->glDrawArrays(GL_TRIANGLES, 0, 12*3);

    PVM = PVM2;

    //


    gl->glUniformMatrix4fv(gl->glGetUniformLocation(id_sencilni_program, "PVM"), 1, GL_FALSE, glm::value_ptr(PVM));
    gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 1,dodajZelenoBarvo,0,0);
    gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 0,0.7,0.5,0);

    gl->glBindBuffer(GL_ARRAY_BUFFER, id_plane);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    gl->glDrawArrays(GL_TRIANGLES, 0, 6);


    gl->glBindBuffer(GL_ARRAY_BUFFER, id_buffer_trikotnik);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);


    float m = 0.0f;

    for(float i = -1.0f; i < 2.0f; i++)

        for(float j = -1.0f; j < 2.0f; j++){

            Model = glm::translate(glm::vec3(i + translateX, 0.0f + translateY, j+translateZ)) * glm::rotate(glm::radians(rotX), rotateMatrix);
            PVM = Projection * View * Model;

            gl->glUniformMatrix4fv(gl->glGetUniformLocation(id_sencilni_program, "PVM"), 1, GL_FALSE, glm::value_ptr(PVM));
            gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 0+m,0+m*0.2,0+m*0.4,0);
            gl->glDrawArrays(GL_TRIANGLES, 0, 12*3);


            PVM = PVM2;

            update();
            m+=0.3;

        }







    const unsigned int err = gl->glGetError();
    if (err != 0) {
        std::cerr << "OpenGL napaka: " << err << std::endl;
    }
}


void WidgetOpenGLDraw::NarediNekajRotacija(){
    makeCurrent();// če bomo tukaj delali z OpenGL, je predtem potrebno klicati tole funkcijo
    // s tem povemo, da izvajamo OpenGL nad to površino (aplikacije imajo lahko več površin za izris!)
    // dialogi za odpiranje datotek imajo svoje površine in s tem svoj kontext!
    // http://doc.qt.io/qt-5/qopenglwidget.html#makeCurrent

    // lahko naložimo nove podatke
    //gl->glBufferData

    rotX+=36.0f;
    update();
}



void WidgetOpenGLDraw::NarediNekajDodajZelenoBarvo(){
    makeCurrent();// če bomo tukaj delali z OpenGL, je predtem potrebno klicati tole funkcijo
    // s tem povemo, da izvajamo OpenGL nad to površino (aplikacije imajo lahko več površin za izris!)
    // dialogi za odpiranje datotek imajo svoje površine in s tem svoj kontext!
    // http://doc.qt.io/qt-5/qopenglwidget.html#makeCurrent

    // lahko naložimo nove podatke
    //gl->glBufferData

    dodajZelenoBarvo+=0.1;
    update();
}



void WidgetOpenGLDraw::keyPressEvent(QKeyEvent *e){


    switch (e->key()) {
    case Qt::Key::Key_X:
        fy = false;
        fz = false;
        fx = true;
        rotX+=36.0f;
        update();
        break;
    case Qt::Key::Key_Y:
        fz = false;
        fx = false;
        fy = true;
        rotX+=36.0f;
        update();
        break;
    case Qt::Key::Key_Z:
        fx = false;
        fy = false;
        fz = true;
        rotX+=36.0f;
        update();
        break;
    case Qt::Key::Key_W:
        //fz = true;
        positionX-=0.1f;
        update();
        break;
    case Qt::Key::Key_S:
        //fz = true;
        positionX+=0.1f;
        update();
        break;
    case Qt::Key::Key_Left:
        //fz = true;
        translateZ+=0.1f;
        update();
        break;
    case Qt::Key::Key_Right:
        //fz = true;
        translateZ-=0.1f;
        update();
        break;
    case Qt::Key::Key_Down:
        //fz = true;
        translateY-=0.1f;
        update();
        break;
    case Qt::Key::Key_Up:
        //fz = true;
        translateY+=0.1f;
        update();
        break;
    case Qt::Key::Key_I:
        //fz = true;
        translateX-=0.1f;
        update();
        break;
    case Qt::Key::Key_K:
        //fz = true;
        translateX+=0.1f;
        update();
        break;
    case Qt::Key::Key_O:
        if(isOrtho){
            isOrtho = false;
        }else{
            isOrtho = true;
        }

        update();
        break;



    }
    update();

}

void WidgetOpenGLDraw::mouseMoveEvent(QMouseEvent *event){
    mosX = event->pos().x();
    mosY = event->pos().y();

   // update();
}


