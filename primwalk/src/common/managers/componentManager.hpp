#pragma once

// primwalk
#include "../../core.hpp"
#include "../components/componentArray.hpp"
#include "../components/component.hpp"

// std
#include <cassert>
#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace pw {
	class PW_API ComponentManager {
	public:
		ComponentManager() = default;
		~ComponentManager() = default;

		template<typename T>
		void registerComponent() {
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) == m_ComponentTypes.end() && "ERROR: Component type already registered!");

			m_ComponentTypes.insert({ typeName, m_NextComponentType });
			m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

			m_NextComponentType++;
		}

		template<typename T>
		component_type getComponentType() {
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered before use!");

			return m_ComponentTypes[typeName];
		}

		template<typename T, typename... Args>
		T& addComponent(entity_id entity, Args&&... args) {
			getComponentArray<T>()->insert(entity, std::forward<Args>(args)...);
			return getComponent<T>(entity);
		}

		template<typename T>
		void removeComponent(entity_id entity) {
			getComponentArray<T>()->remove(entity);
		}

		template<typename T>
		T& getComponent(entity_id entity) {
			T* component = getComponentArray<T>()->get(entity);
			assert(component != nullptr && "Can not get non-existent component!");

			return *component;
		}

		template<typename T>
		bool hasComponent(entity_id entity) {
			return getComponentArray<T>()->get(entity) != nullptr;
		}

		void entityDestroyed(entity_id entity);

	private:
		std::unordered_map<std::string_view, component_type> m_ComponentTypes{};
		std::unordered_map<std::string_view, std::shared_ptr<IComponentArray>> m_ComponentArrays{};
		component_type m_NextComponentType{};

		template<typename T>
		std::shared_ptr<ComponentArray<T>> getComponentArray() {
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "ERROR: Component not registered before use!");

			return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
		}
	};
}