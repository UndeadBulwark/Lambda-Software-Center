#pragma once

#include "Package.h"
#include <QString>
#include <algorithm>

inline int searchRelevanceScore(const Package &pkg, const QString &query)
{
    QString ql = query.toLower();
    QString nl = pkg.name.toLower();
    if (nl == ql)
        return 1000; // exact match
    if (nl.startsWith(ql))
        return 900; // prefix match
    if (nl.contains(ql))
        return 800; // contains in name
    if (pkg.description.toLower().contains(ql))
        return 100; // description only
    return 0;
}

inline void sortPackagesBySearchRelevance(QList<Package> &results, const QString &query)
{
    std::sort(results.begin(), results.end(),
              [&](const Package &a, const Package &b) {
                  int sa = searchRelevanceScore(a, query);
                  int sb = searchRelevanceScore(b, query);
                  if (sa != sb)
                      return sa > sb; // higher score first
                  return a.name.localeAwareCompare(b.name) < 0; // alphabetical tie-break
              });
}
