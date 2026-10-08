#pragma once
#include <QString>
#include <vector>

namespace gpm {
// Parse the single chord stored by the recorder and legacy JSON. Empty means invalid.
std::vector<int> ParseShortcut(const QString& text);
}
