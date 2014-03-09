#pragma once

#include "serialization/reflection.hpp"
#include "util/ObjectPool.hpp"

template <typename T>
struct SerializationTypeConfig {
	ObjectPool<T>* pool = nullptr;
};

template <typename T>
SerializationTypeConfig<T>& getTypeConfig() {
	static SerializationTypeConfig<T> config;
	return config;
}

template <typename T>
struct SerializationPool {
	ObjectPool<T>* previous_pool;

	SerializationPool(ObjectPool<T>* new_pool) {
		auto& type_config = getTypeConfig<T>();
		previous_pool = type_config.pool;
		type_config.pool = new_pool;
	}

	~SerializationPool() {
		getTypeConfig<T>().pool = previous_pool;
	}
};

extern TypeDatabase type_database;
void register_types();
