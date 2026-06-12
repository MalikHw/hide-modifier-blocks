#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

// WHY
static const std::vector<int> s_hideObjectIDs = {1755, 1813, 1829, 1859, 2866};

class $modify(LevelEditorLayer) {
    struct Fields {
        std::unordered_map<int, bool> prevInvisible;
    };

    void hideMatchingObjects() {
        auto objs = this->getAllObjects();
        if (!objs) return;

        for (size_t i = 0; i < objs->count(); ++i) {
            auto obj = static_cast<GameObject*>(objs->objectAtIndex(i));
            if (!obj) continue;

            int oid = obj->m_objectID;
            if (std::find(s_hideObjectIDs.begin(), s_hideObjectIDs.end(), oid) != s_hideObjectIDs.end()) {
                int uid = obj->m_uniqueID;
                if (m_fields->prevInvisible.find(uid) == m_fields->prevInvisible.end()) {
                    // save previous invisible state
                    bool prev = obj->m_isInvisible;
                    m_fields->prevInvisible[uid] = prev;
                }
                // mark invisible for editor/playtest handling
                obj->m_isInvisible = true;
            }
        }
    }

    void restoreSavedObjects() {
        // restore objs by their id
        for (auto it = m_fields->prevInvisible.begin(); it != m_fields->prevInvisible.end(); ++it) {
            int uid = it->first;
            bool prev = it->second;
            auto obj = this->findGameObject(uid);
            if (!obj) continue;
            obj->m_isInvisible = prev;
        }

        m_fields->prevInvisible.clear();
    }

    void onPlaytest() {
        LevelEditorLayer::onPlaytest();
        hideMatchingObjects();
    }

#ifndef GEODE_IS_WINDOWS
    void onPausePlaytest() {
        restoreSavedObjects();
        LevelEditorLayer::onPausePlaytest();
    }
#endif

#ifndef GEODE_IS_WINDOWS
    void onResumePlaytest() {
        LevelEditorLayer::onResumePlaytest();
        hideMatchingObjects();
    }
#endif

    void onStopPlaytest() {
        restoreSavedObjects();
        LevelEditorLayer::onStopPlaytest();
    }
};
