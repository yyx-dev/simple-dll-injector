#pragma once

#include <QString>
#include <QList>

#include "utils.h"
#include "injector.h"

namespace render {

struct ProcessInfo {
    quint32 pid;
    QString name;
};

inline QList<ProcessInfo> GetAllProcesses() {
    auto v = utils::GetAllProcesses();
    QList<ProcessInfo> l;
    for (auto& e : v) {
        l.append({e.pid, QString::fromStdWString(e.name)});
    }
    return l;
}

inline bool IsProcess32Bits(quint32 pid) {
    return utils::IsProcess32Bits(pid);
}

inline bool RemoteThreadInject(quint32 pid, const QString& dllPath) {
    return inject::RemoteThreadInject(pid, dllPath.toStdString());
}

}