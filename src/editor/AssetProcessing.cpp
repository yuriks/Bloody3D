#include "AssetProcessing.hpp"

#include "MeshCooker.hpp"

#include <iostream>

namespace editor {

bool asset_processing(int argc, char *argv[]) {
	if (argc == 0) {
		std::cerr << "[AssetProcessing] Nothing to do." << std::endl;
		return true;
	}

	while (argc > 0) {
		if (std::strcmp(argv[0], "-m") == 0) {
			argc -= 1; argv += 1;
			if (!cook_mesh(argc, argv)) {
				return false;
			}
		} else {
			std::cerr << "[AssetProcessing] Unknown option: " << argv[0] << std::endl;
			return false;
		}
	}

	return true;
}

} // namespace editor
