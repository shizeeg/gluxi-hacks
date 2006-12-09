#ifndef COMMON_H
#define COMMON_H

#include <QString>

QString secsToString(int secs);
QString version();
bool isSafeArg(const QString&);
QString getValue(const QString&s, const QString& exp, bool last=false);
QString removeHtml(const QString&);

#endif
