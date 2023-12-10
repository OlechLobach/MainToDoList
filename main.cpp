
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ToDoList window;
    window.show();

    return a.exec();
}

#include "main.moc"
