#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include<algorithm>



class ToDoList : public QWidget {
    Q_OBJECT

public:
    explicit ToDoList(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("ToDoList");

        layout = new QVBoxLayout(this);
        todoTable = new QTableWidget(this);
        todoTable->setColumnCount(4);
        QStringList headers;
        headers << "Назва" << "Опис" << "Дата виконання" << "Статус";
        todoTable->setHorizontalHeaderLabels(headers);
        layout->addWidget(todoTable);

        QLabel *titleLabel = new QLabel("Назва завдання:", this);
        layout->addWidget(titleLabel);

        titleField = new QLineEdit(this);
        layout->addWidget(titleField);

        QLabel *descriptionLabel = new QLabel("Опис завдання:", this);
        layout->addWidget(descriptionLabel);

        descriptionField = new QLineEdit(this);
        layout->addWidget(descriptionField);

        dueDateEdit = new QDateEdit(QDate::currentDate(), this);
        layout->addWidget(dueDateEdit);

        addButton = new QPushButton("Додати", this);
        layout->addWidget(addButton);

        editButton = new QPushButton("Редагувати", this);
        layout->addWidget(editButton);

       searchButton = new QPushButton("Пошук", this);
         layout->addWidget(searchButton);

        deleteButton = new QPushButton("Видалити", this);
        layout->addWidget(deleteButton);

        completeButton = new QPushButton("Виконано", this);
        layout->addWidget(completeButton);

        sortByPriorityButton = new QPushButton("Сортувати за пріоритетом", this);
        layout->addWidget(sortByPriorityButton);

        sortByDateButton = new QPushButton("Сортувати за датою", this);
        layout->addWidget(sortByDateButton);

        sortByNameButton = new QPushButton("Сортувати за назвою", this);
        layout->addWidget(sortByNameButton);

        connect(addButton, &QPushButton::clicked, this, &ToDoList::addItemToList);
        connect(editButton, &QPushButton::clicked, this, &ToDoList::editSelectedItem);
        connect(titleField, &QLineEdit::returnPressed, this, &ToDoList::saveEditedItem);
        connect(descriptionField, &QLineEdit::returnPressed, this, &ToDoList::saveEditedItem);
        connect(deleteButton, &QPushButton::clicked, this, &ToDoList::deleteSelectedItems);
        connect(completeButton, &QPushButton::clicked, this, &ToDoList::markItemsAsCompleted);
        connect(searchButton, &QPushButton::clicked, this, &ToDoList::searchItems);
        connect(sortByPriorityButton, &QPushButton::clicked, this, &ToDoList::sortItemsByPriority);
        connect(sortByDateButton, &QPushButton::clicked, this, &ToDoList::sortItemsByDate);
        connect(sortByNameButton, &QPushButton::clicked, this, &ToDoList::sortItemsByName);

        setLayout(layout);
        connect(qApp, &QApplication::aboutToQuit, this, &ToDoList::saveNotesToFile);
        loadNotesFromFile();
    }



private slots:

    void searchItems() {
        QString searchText = titleField->text(); // Текст, який ми шукаємо

        for (int row = 0; row < todoTable->rowCount(); ++row) {
            bool matchFound = false;
            for (int col = 0; col < todoTable->columnCount(); ++col) {
                QTableWidgetItem *item = todoTable->item(row, col);
                if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                    matchFound = true;
                    break;
                }
            }
            todoTable->setRowHidden(row, !matchFound);
        }
    }


    void addItemToList() {
        QString titleText = titleField->text();
        QString descriptionText = descriptionField->text();
        QDate dueDate = dueDateEdit->date();
        QString status = "В процесі";

        if (!titleText.isEmpty() && !descriptionText.isEmpty()) {
            int row = todoTable->rowCount();
            todoTable->insertRow(row);
            todoTable->setItem(row, 0, new QTableWidgetItem(titleText));
            todoTable->setItem(row, 1, new QTableWidgetItem(descriptionText));
            todoTable->setItem(row, 2, new QTableWidgetItem(dueDate.toString("yyyy-MM-dd")));

            // Перевірка, чи минула дата виконання
            QDate currentDate = QDate::currentDate();
            if (dueDate < currentDate) {
                status = "Просрочено";
            }

            todoTable->setItem(row, 3, new QTableWidgetItem(status));

            titleField->clear();
            descriptionField->clear();
        }
    }

    void editSelectedItem() {
        QList<QTableWidgetItem*> selectedItems = todoTable->selectedItems();
        if (!selectedItems.isEmpty()) {
            int selectedRow = selectedItems.first()->row();
            titleField->setText(todoTable->item(selectedRow, 0)->text());
            descriptionField->setText(todoTable->item(selectedRow, 1)->text());
            dueDateEdit->setDate(QDate::fromString(todoTable->item(selectedRow, 2)->text(), "yyyy-MM-dd"));

            // Встановлення статусу редагування та номера редагуємого рядка
            editingMode = true;
            editedRow = selectedRow;
        }
    }

    void saveEditedItem() {
        if (editingMode) {
            QString titleText = titleField->text();
            QString descriptionText = descriptionField->text();
            QDate dueDate = dueDateEdit->date();
            QString status = "В процесі";

            if (!titleText.isEmpty() && !descriptionText.isEmpty()) {
                todoTable->item(editedRow, 0)->setText(titleText);
                todoTable->item(editedRow, 1)->setText(descriptionText);
                todoTable->item(editedRow, 2)->setText(dueDate.toString("yyyy-MM-dd"));

                // Update the status based on the due date logic
                QDate currentDate = QDate::currentDate();
                if (dueDate < currentDate) {
                    status = "Просрочено";
                }

                todoTable->item(editedRow, 3)->setText(status);

                editingMode = false;  // Mark editing mode as finished
                editedRow = -1;  // Clear the edited row index

                // Clear fields after editing
                titleField->clear();
                descriptionField->clear();
            }
        }
    }




    void deleteSelectedItems() {
        QList<QTableWidgetItem*> items = todoTable->selectedItems();
        QList<int> rowsToDelete;

        // Збираємо унікальні індекси рядків для видалення
        for (auto item : items) {
            int row = item->row();
            if (!rowsToDelete.contains(row)) {
                rowsToDelete.append(row);
            }
        }

        // Видаляємо рядки у зворотньому порядку, щоб уникнути проблем з індексами
        std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());
        for (int row : rowsToDelete) {
            todoTable->removeRow(row);
        }
    }

    void markItemsAsCompleted() {
        QList<QTableWidgetItem*> items = todoTable->selectedItems();
        for (auto item : items) {
            int column = item->column();
            if (column == 3) { // Перевіряємо, чи обраний елемент у стовпчику "Статус"
                QTableWidgetItem* statusItem = todoTable->item(item->row(), column);
                QString dueDateString = statusItem->text();

                QDate dueDate = QDate::fromString(dueDateString, "yyyy-MM-dd");
                QDate currentDate = QDate::currentDate();

                if (dueDate.isValid() && currentDate > dueDate) {
                    statusItem->setText("Просрочено");
                } else {
                    statusItem->setText("Виконано");
                }
            }
        }
    }

    void sortItemsByPriority() {
        const int priorityColumnIndex = 0; // Оновіть це значення за потреби
        todoTable->sortItems(priorityColumnIndex, Qt::AscendingOrder);
    }

    void sortItemsByDate() {
        const int dateColumnIndex = 2; // Оновіть це значення за потреби
        todoTable->sortItems(dateColumnIndex, Qt::AscendingOrder);
    }

    void sortItemsByName() {
        const int nameColumnIndex = 1; // Оновіть це значення за потреби
        todoTable->sortItems(nameColumnIndex, Qt::AscendingOrder);
    }


    void saveNotesToFile() {
        QFile file("notes.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < todoTable->rowCount(); ++i) {
                QString text = todoTable->item(i, 0)->text();
                QString dueDate = todoTable->item(i, 1)->text();
                QString description = todoTable->item(i, 2)->text();
                QString status = todoTable->item(i, 3)->text(); // Save status

                out << text << "|" << dueDate << "|" << description << "|" << status << "\n";
            }
            file.close();
        }
    }

    void loadNotesFromFile() {
        QFile file("notes.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split("|");
                if (parts.size() >= 4) {
                    QString text = parts[0];
                    QString dueDate = parts[1];
                    QString description = parts[2];
                    QString status = parts[3]; // Load status

                    int row = todoTable->rowCount();
                    todoTable->insertRow(row);
                    todoTable->setItem(row, 0, new QTableWidgetItem(text));
                    todoTable->setItem(row, 1, new QTableWidgetItem(dueDate));
                    todoTable->setItem(row, 2, new QTableWidgetItem(description));
                    todoTable->setItem(row, 3, new QTableWidgetItem(status));
                }
            }
            file.close();
        }
    }




private:
    QVBoxLayout *layout;
    QTableWidget *todoTable;
    QLineEdit *titleField;
    QLineEdit *descriptionField;
    QDateEdit *dueDateEdit;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *searchButton;
    QPushButton *deleteButton;
    QPushButton *completeButton;
    QPushButton *sortByPriorityButton;
    QPushButton *sortByDateButton;
    QPushButton *sortByNameButton;
    bool editingMode = false;  // Прапорець режиму редагування
    int editedRow = -1;  // Номер рядка, який редагується

};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ToDoList window;
    window.show();

    return a.exec();
}

#include "main.moc"
