"""Allow running the PCG pipeline as ``python -m pcg_pipeline``."""

import sys

from .batch_generate import main

sys.exit(main())
