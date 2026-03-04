# Codename: Subspace - Makefile
# Provides easy commands for common development tasks

# Detect OS
ifeq ($(OS),Windows_NT)
    RM := del /Q
    RMDIR := rmdir /S /Q
else
    RM := rm -f
    RMDIR := rm -rf
endif

# Logging setup
LOG_DIR := logs
TIMESTAMP := $(shell date '+%Y%m%d_%H%M%S')
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Default target
.DEFAULT_GOAL := help

.PHONY: help
help: ## Show this help message
	@echo "Codename: Subspace - Development Commands"
	@echo "=========================================="
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""

# ---------------------------------------------------------------------------
# C++ Engine (CMake)
# ---------------------------------------------------------------------------

.PHONY: build
build: ## Build C++ engine (Release)
	@mkdir -p $(LOG_DIR)
	@mkdir -p engine/build
	(cd engine/build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release -j$(NPROC)) 2>&1 | tee $(LOG_DIR)/build_$(TIMESTAMP).log

.PHONY: build-debug
build-debug: ## Build C++ engine (Debug)
	@mkdir -p $(LOG_DIR)
	@mkdir -p engine/build
	(cd engine/build && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build . --config Debug -j$(NPROC)) 2>&1 | tee $(LOG_DIR)/build-debug_$(TIMESTAMP).log

.PHONY: build-engine
build-engine: ## Build C++ engine library only (no game executable)
	@mkdir -p $(LOG_DIR)
	@mkdir -p engine/build
	(cd engine/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DSUBSPACE_BUILD_TESTS=OFF && cmake --build . --config Release --target subspace_engine -j$(NPROC)) 2>&1 | tee $(LOG_DIR)/build-engine_$(TIMESTAMP).log

# ---------------------------------------------------------------------------
# C# Prototype (dotnet)
# ---------------------------------------------------------------------------

.PHONY: build-csharp
build-csharp: ## Build C# prototype
	@mkdir -p $(LOG_DIR)
	(cd AvorionLike && dotnet restore && dotnet build) 2>&1 | tee $(LOG_DIR)/build-csharp_$(TIMESTAMP).log

.PHONY: run-csharp
run-csharp: ## Run C# prototype
	cd AvorionLike && dotnet run

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------

.PHONY: test
test: test-engine ## Run all tests

.PHONY: test-engine
test-engine: ## Build and run C++ engine tests
	@mkdir -p engine/build
	cd engine/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DSUBSPACE_BUILD_TESTS=ON && cmake --build . --config Release -j$(NPROC)
	cd engine/build && ./subspace_tests

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

.PHONY: validate
validate: ## Validate all GameData JSON files
	python3 tools/validate_json.py

.PHONY: validate-verbose
validate-verbose: ## Validate GameData JSON with verbose output
	python3 tools/validate_json.py --verbose

.PHONY: contract-scan
contract-scan: ## Scan C++ engine for contract violations
	python3 tools/contract_scan.py

# ---------------------------------------------------------------------------
# Maintenance
# ---------------------------------------------------------------------------

.PHONY: clean
clean: ## Clean all build artifacts
	$(RMDIR) engine/build 2>/dev/null || true
	$(RMDIR) logs 2>/dev/null || true
	find . -type d -name "__pycache__" -exec $(RMDIR) {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true

.PHONY: check-deps
check-deps: ## Check if build dependencies are installed
	@echo "Checking build dependencies..."
	@command -v cmake >/dev/null 2>&1 && echo "  ✓ CMake ($$(cmake --version | head -1))" || echo "  ✗ CMake not found"
	@command -v g++ >/dev/null 2>&1 && echo "  ✓ g++" || (command -v clang++ >/dev/null 2>&1 && echo "  ✓ clang++" || echo "  ✗ No C++ compiler found")
	@command -v dotnet >/dev/null 2>&1 && echo "  ✓ .NET SDK ($$(dotnet --version))" || echo "  ✗ .NET SDK not found"
	@command -v python3 >/dev/null 2>&1 && echo "  ✓ Python3 ($$(python3 --version))" || echo "  ✗ Python3 not found"
	@command -v git >/dev/null 2>&1 && echo "  ✓ git ($$(git --version))" || echo "  ✗ git not found"

.PHONY: docs
docs: ## Show documentation locations
	@echo "Documentation is in docs/ folder:"
	@echo ""
	@echo "  docs/architecture/  - Architecture documentation"
	@echo "  docs/design/        - Design documents"
	@echo "  docs/guides/        - Build & setup guides"
	@echo "  docs/implementation/- Implementation details"
	@echo ""
	@ls -1 docs/*.md 2>/dev/null || true

.PHONY: setup
setup: ## Run the full setup script
	./scripts/setup.sh

.PHONY: all
all: clean build build-csharp ## Clean and build everything
