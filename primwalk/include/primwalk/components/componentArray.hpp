#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/component.hpp"

// std
#include <array>
#include <cassert>
#include <unordered_map>

namespace pw {
  class PW_API IComponentArray
  {
  public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(entity_id entity) = 0;
  };

  template <typename T>
  class PW_API ComponentArray : public IComponentArray {
  public:
    template <typename... Args>
    void insert(entity_id entity, Args&&... args) {
      assert(m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end() && "ERROR: Duplicate component added to entity!");

      size_t newIndex = m_Size;
      m_EntityToIndexMap[entity] = newIndex;
      m_IndexToEntityMap[newIndex] = entity;
      m_ComponentArray[newIndex] = T(args...);
      m_Size++;
    }

    void remove(entity_id entity) {
      assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "ERROR: Removing non-existent component!");

      size_t indexOfRemovedEntity = m_EntityToIndexMap[entity];
      size_t indexOfLastElement = m_Size - 1;
      m_ComponentArray[indexOfRemovedEntity] = m_ComponentArray[indexOfLastElement];

      entity_id entityOfLastElement = m_IndexToEntityMap[indexOfLastElement];
      m_EntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
      m_IndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

      m_EntityToIndexMap.erase(entity);
      m_IndexToEntityMap.erase(indexOfLastElement);

      m_Size--;
    }

    T& get(entity_id entity) {
      assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "ERROR: Can not retrieve non-existent component!");

      // Return a reference to the entity's component
      return m_ComponentArray[m_EntityToIndexMap[entity]];
    }

    void entityDestroyed(entity_id entity) override {
      if (m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end())
      {
        remove(entity);
      }
    }

  private:
    std::array<T, MAX_ENTITIES> m_ComponentArray; // packed array
    std::unordered_map<entity_id, size_t> m_EntityToIndexMap;
    std::unordered_map<size_t, entity_id> m_IndexToEntityMap;
    size_t m_Size = 0;
  };
}