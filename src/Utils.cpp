#include "Utils.hpp"

void assertNSError(NS::Error *pErr) {
    if (pErr != nullptr) {
        fprintf(stderr, "%s\n", pErr->localizedDescription()->cString(NS::UTF8StringEncoding));
        exit(-1);
    }
}