#ifndef DATASOURCE_H
#define DATASOURCE_H

struct DataSource {
    DataSource() = default;
    virtual ~DataSource() = default;
    virtual operator bool() const noexcept = 0;
};

#endif // DATASOURCE_H
