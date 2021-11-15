#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
	ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_actionTest_triggered(){
	QMessageBox::information(this,"Test title","Test test");
}

void MainWindow::on_pushButtonRotacija_clicked(){
    ui->widget->NarediNekajRotacija();
}

void MainWindow::on_pushButtonZelenaBarva_clicked(){
    ui->widget->NarediNekajDodajZelenoBarvo();
}
