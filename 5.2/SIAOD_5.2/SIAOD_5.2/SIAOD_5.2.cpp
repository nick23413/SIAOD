#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include<algorithm>;

using namespace std;

struct Table {
    int trainNumber; // Номер поезда
    long offset;     // Смещение в файле

    Table(int trainNumber, long offset) : trainNumber(trainNumber), offset(offset) {}
};

struct Record {
    int trainNumber;
    string departurePoint;
    string destinationPoint;
    string departureTime;
};

void text_file(const string& filename, int count) {
    ofstream outfile(filename);
    if (!outfile) {
        cerr << "Error with opening" << endl;
        return;
    }

    vector<string> departurePoints = {
        "Moscow", "Saint_Petersburg", "Novosibirsk", "Yekaterinburg", "Kazan",
        "Nizhny_Novgorod", "Chelyabinsk", "Samara", "Omsk", "Rostov-on-Don"
    };

    vector<string> destinationPoints = {
        "Moscow", "Saint_Petersburg", "Novosibirsk", "Yekaterinburg", "Kazan",
        "Nizhny_Novgorod", "Chelyabinsk", "Samara", "Omsk", "Rostov-on-Don"
    };

    vector<string> departureTimes = {
        "08:00", "09:30", "10:45", "12:15", "13:30",
        "14:45", "16:00", "17:15", "18:30", "19:45"
    };

    for (int i = 0; i < count; ++i) {
        Record record;
        record.trainNumber = 1000 + rand() % 9000;
        record.departurePoint = departurePoints[rand() % departurePoints.size()];
        record.destinationPoint = destinationPoints[rand() % destinationPoints.size()];
        record.departureTime = departureTimes[rand() % departureTimes.size()];

        outfile << record.trainNumber << " "
                << record.departurePoint << " "
                << record.destinationPoint << " "
                << record.departureTime << endl;
    }

    outfile.close();
}

void text_to_bin_file(const string& text_filename, const string& bin_filename) {
    ifstream infile(text_filename);
    ofstream outfile(bin_filename, ios::binary);

    if (!outfile || !infile) {
        cerr << "Error with opening" << endl;
        return;
    }

    int trainNumber;
    string departurePoint, destinationPoint, departureTime;

    while (infile >> trainNumber >> departurePoint >> destinationPoint >> departureTime) {
        long dpLength = departurePoint.size();
        long dnLength = destinationPoint.size();
        long dtLength = departureTime.size();

        outfile.write(reinterpret_cast<char*>(&trainNumber), sizeof(trainNumber));
        outfile.write(reinterpret_cast<char*>(&dpLength), sizeof(dpLength));
        outfile.write(departurePoint.data(), dpLength);
        outfile.write(reinterpret_cast<char*>(&dnLength), sizeof(dnLength));
        outfile.write(destinationPoint.data(), dnLength);
        outfile.write(reinterpret_cast<char*>(&dtLength), sizeof(dtLength));
        outfile.write(departureTime.data(), dtLength);
    }

    infile.close();
    outfile.close();
}

void index_table(const string& filename, vector<Table>& index) {
    ifstream infile(filename, ios::binary);
    if (!infile) {
        cerr << "Error with opening " << filename << endl;
        return;
    }

    int trainNumber;
    long dpLength, dnLength, dtLength;
    long offset = 0;

    // Читаем файл построчно
    while (infile.read(reinterpret_cast<char*>(&trainNumber), sizeof(trainNumber))) {
        // Сохраняем текущее смещение (начало записи)
        offset = infile.tellg() - static_cast<streamoff>(sizeof(trainNumber));

        // Читаем длины строк
        infile.read(reinterpret_cast<char*>(&dpLength), sizeof(dpLength));
        infile.seekg(dpLength, ios::cur); // Пропускаем строку departurePoint

        infile.read(reinterpret_cast<char*>(&dnLength), sizeof(dnLength));
        infile.seekg(dnLength, ios::cur); // Пропускаем строку destinationPoint

        infile.read(reinterpret_cast<char*>(&dtLength), sizeof(dtLength));
        infile.seekg(dtLength, ios::cur); // Пропускаем строку departureTime

        // Добавляем номер поезда и смещение в индексную таблицу
        index.push_back(Table(trainNumber, offset));
    }

    infile.close();

    // Сортируем индексную таблицу по номеру поезда
    sort(index.begin(), index.end(), [](const Table& a, const Table& b) {
        return a.trainNumber < b.trainNumber;
        });
}

bool read_by_offset(const string& filename, long offset, Record& result) {
    ifstream infile(filename, ios::binary);
    if (!infile) {
        cerr << "Error with opening " << filename << endl;
        return false;
    }

    // Перемещаем указатель чтения на указанное смещение
    infile.seekg(offset);

    long dpLength, dnLength, dtLength;

    // Читаем номер поезда
    infile.read(reinterpret_cast<char*>(&result.trainNumber), sizeof(result.trainNumber));

    // Читаем длину и содержимое departurePoint
    infile.read(reinterpret_cast<char*>(&dpLength), sizeof(dpLength));
    result.departurePoint.resize(dpLength);
    infile.read(&result.departurePoint[0], dpLength);

    // Читаем длину и содержимое destinationPoint
    infile.read(reinterpret_cast<char*>(&dnLength), sizeof(dnLength));
    result.destinationPoint.resize(dnLength);
    infile.read(&result.destinationPoint[0], dnLength);

    // Читаем длину и содержимое departureTime
    infile.read(reinterpret_cast<char*>(&dtLength), sizeof(dtLength));
    result.departureTime.resize(dtLength);
    infile.read(&result.departureTime[0], dtLength);

    infile.close();
    return true;
}

bool Linear_search(const string& filename, int key, Record& result) {
    ifstream infile(filename, ios::binary);
    if (!infile) {
        cerr << "Error with opening" << endl;
        return false;
    }

    int trainNumber;
    long dpLength, dnLength, dtLength;
    string departurePoint, destinationPoint, departureTime;

    while (infile.read(reinterpret_cast<char*>(&trainNumber), sizeof(trainNumber))) {
        infile.read(reinterpret_cast<char*>(&dpLength), sizeof(dpLength));
        departurePoint.resize(dpLength);
        infile.read(&departurePoint[0], dpLength);

        infile.read(reinterpret_cast<char*>(&dnLength), sizeof(dnLength));
        destinationPoint.resize(dnLength);
        infile.read(&destinationPoint[0], dnLength);

        infile.read(reinterpret_cast<char*>(&dtLength), sizeof(dtLength));
        departureTime.resize(dtLength);
        infile.read(&departureTime[0], dtLength);

        if (trainNumber == key) {
            result.trainNumber = trainNumber;
            result.departurePoint = departurePoint;
            result.destinationPoint = destinationPoint;
            result.departureTime = departureTime;
            cout << "Entry is found: " << result.trainNumber << " "
                 << result.departurePoint << " "
                 << result.destinationPoint << " "
                 << result.departureTime << endl;
            infile.close();
            return true;
        }
    }

    infile.close();
    cout << "Entry with train number " << key << " is not found" << endl;
    return false;
}

bool search_by_index(const string& filename, const vector<Table>& index, int key, Record& result) {
    // Используем бинарный поиск для нахождения записи в индексной таблице
    auto it = lower_bound(index.begin(), index.end(), key, [](const Table& a, int key) {
        return a.trainNumber < key;
        });

    if (it == index.end() || it->trainNumber != key) {
        cout << "Entry with train number " << key << " is not found" << endl;
        return false;
    }

    // Читаем запись по найденному смещению
    if (!read_by_offset(filename, it->offset, result)) {
        cerr << "Error reading record from file" << endl;
        return false;
    }

    cout << "Entry is found: " << result.trainNumber << " "
        << result.departurePoint << " "
        << result.destinationPoint << " "
        << result.departureTime << endl;
    return true;
}

/*
int main() {
    vector<int> counts = { 100, 1000, 10000 };
    vector<string> text_filename = { "trains_100.txt", "trains_1000.txt", "trains_10000.txt" };
    vector<string> bin_filename = { "trains_100.bin", "trains_1000.bin", "trains_10000.bin" };

    for (int i = 0; i < counts.size(); ++i) {
        text_file(text_filename[i], counts[i]);
        text_to_bin_file(text_filename[i], bin_filename[i]);
    }

    int key;
    cout << "Enter a train number:" << endl;
    cin >> key;

    Record result;
    chrono::duration<double> duration;

    for (int i = 0; i < counts.size(); ++i) {
        auto start_time = chrono::high_resolution_clock::now();
        Linear_search(bin_filename[i], key, result);
        auto end_time = chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        cout << "Duration of searching: " << duration.count() << " seconds" << endl;
    }

    return 0;
}
*/

/*
void print_binary_file(const string& filename) {
    ifstream infile(filename, ios::binary);
    if (!infile) {
        cerr << "Error opening binary file: " << filename << endl;
        return;
    }

    int trainNumber;
    long dpLength, dnLength, dtLength;
    string departurePoint, destinationPoint, departureTime;

    while (infile.read(reinterpret_cast<char*>(&trainNumber), sizeof(trainNumber))) {
        // Читаем длину и содержимое departurePoint
        infile.read(reinterpret_cast<char*>(&dpLength), sizeof(dpLength));
        departurePoint.resize(dpLength);
        infile.read(&departurePoint[0], dpLength);

        // Читаем длину и содержимое destinationPoint
        infile.read(reinterpret_cast<char*>(&dnLength), sizeof(dnLength));
        destinationPoint.resize(dnLength);
        infile.read(&destinationPoint[0], dnLength);

        // Читаем длину и содержимое departureTime
        infile.read(reinterpret_cast<char*>(&dtLength), sizeof(dtLength));
        departureTime.resize(dtLength);
        infile.read(&departureTime[0], dtLength);

        // Выводим данные
        cout << "Binary file content: " << trainNumber << " "
            << departurePoint << " "
            << destinationPoint << " "
            << departureTime << endl;
    }

    infile.close();
}
*/

int main() {
    //srand(static_cast<unsigned>(time(0))); // Инициализация генератора случайных чисел

    vector<int> counts = { 100, 1000, 10000 };
    vector<string> text_filename = { "trains_100.txt", "trains_1000.txt", "trains_10000.txt" };
    vector<string> bin_filename = { "trains_100.bin", "trains_1000.bin", "trains_10000.bin" };

    for (int i = 0; i < counts.size(); ++i) {
        text_file(text_filename[i], counts[i]);
        text_to_bin_file(text_filename[i], bin_filename[i]);
    }

    int key;
    cout << "Enter a train number:" << endl;
    cin >> key;

    Record result;
    chrono::duration<double> duration;

    for (int i = 0; i < counts.size(); ++i) {
        vector<Table> index;
        index_table(bin_filename[i], index); // Создаем индексную таблицу

        auto start_time = chrono::high_resolution_clock::now();
        search_by_index(bin_filename[i], index, key, result); // Поиск по индексной таблице
        auto end_time = chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        cout << "Duration of searching: " << duration.count() << " seconds" << endl;
    }

    return 0;
}