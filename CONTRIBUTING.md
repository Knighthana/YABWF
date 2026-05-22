# Contributing to YABWF

Thank you for your interest in YABWF (Yet Another Boa Web Framework) — a lightweight, single-process HTTP server for embedded Linux systems.

This document outlines the workflow, standards, and expectations for contributing.

## Project Overview

YABWF is forked from Boa webserver (0.94.14) with extensive security fixes, documentation infrastructure, and testing harness. The project targets ARM/Linux and other low-resource embedded platforms.

## Agent-Human Collaboration Model

YABWF uses a structured four-role agent workflow:

| Role | Responsibility |
|------|---------------|
| **arch** | Architecture planning, SPEC creation, task card generation |
| **smith** | Implementation per task card, code changes |
| **probe** | Test writing and verification |
| **audit** | Review and acceptance |

Each task follows the cycle: **arch → smith → probe → audit**. No role skips another's gate.

## SPEC-Driven Development

All code changes **must** be preceded by a corresponding SPEC document in `repo_spec/`.

- The SPEC defines the module's interface, data structures, error codes, and test strategy.
- SPEC documents follow the metadata format specified in `repo_memo/global/DOCUMENT_METADATA.md`.
- For new modules, create the SPEC before any implementation.
- For changes to existing modules, update the SPEC first or confirm the change does not alter the documented interface.

**Exception**: Bug fixes (e.g., buffer overflow, CVE patches) that do not change the public API may proceed with just an updated `CVE_ANALYSIS.md` entry and test case.

## Memory Constraints

YABWF is designed for embedded systems with tight memory budgets.

- All connections share a single process space; no per-request fork/thread model.
- Fixed-size stack buffers are preferred over heap allocation.
- The `request` struct is managed via an object pool to avoid fragmenting the heap.

**Profiling requirement**: Any change involving buffer sizes, struct member additions, or new memory allocations **must** include a profiling report:
  1. Per-connection steady-state memory delta
  2. Peak concurrent RSS delta (including mmap cache and CGI children)
  3. Comparison against baseline

Place the report in `test/` and submit it with the same PR as the code change.

## Code Standards

### Documentation

- All functions **must** have doxygen-style comments (`/** */`) with `@brief`, `@param`, and `@return` tags.
- Return value conventions follow `PATTERNS_ENGINEERING.md` (see `repo_memo/global/PATTERNS_ENGINEERING.md`).
- Buffer bounds and invariants must be documented when not obvious.

### Compilation

- **Zero warnings** is the hard requirement. All code must compile with `-Wall -Werror` (or as close as the existing codebase allows).
- The project uses GNU Autoconf + Make. Run `./configure && make` before submitting.
- For cross-compilation, use `construct.sh` (see its `--help` for available targets).

### Naming

- Functions and variables: `snake_case`
- Macros and enumerations: `UPPER_SNAKE_CASE`
- Global variables are declared in `globals.h` and defined in their respective `.c` files.

## Commit Message Format

```
module: Short description (max 50 chars)

Optional longer description, wrapped at 72 characters.
```

Examples:
- `util.c: Reduce sanitize_log_string buffer to 2KB`
- `cgi_header.c: Reject absolute paths in Location header`
- `hash.c: Clarify four_char_hash operator precedence`

## CVE Fix Process

1. **Reproduce**: Write a PoC script in `test/security/` that demonstrates the vulnerability.
2. **Fix**: Implement the fix with a comment referencing the CVE ID.
3. **Test**: Verify the PoC no longer triggers the vulnerability.
4. **Document**: Update `CVE_ANALYSIS.md` with the fix details, affected versions, and mitigation notes.

## Pull Request Checklist

Before submitting a PR, ensure:

- [ ] SPEC documents exist and are up to date (or a SPEC exemption is documented for bug fixes)
- [ ] Code compiles with zero warnings (`make clean && make`)
- [ ] Existing tests pass (`make test` or manual test suite run)
- [ ] New tests are added for new functionality
- [ ] Memory profiling report is attached for buffer / allocation changes
- [ ] Doxygen comments are complete for new/modified functions
- [ ] Commit messages follow the `module: description` format
- [ ] `CHANGES.md` is updated for significant changes
- [ ] CVE fixes have corresponding entries in `CVE_ANALYSIS.md` and PoC in `test/security/`

## Getting Help

- Read the SPEC documents in `repo_spec/` for module-level design.
- Read the engineering patterns in `repo_memo/global/PATTERNS_ENGINEERING.md`.
- Consult `repo_memo/` for project-level decisions and configuration options.

## License

YABWF is licensed under GPLv2 (same as the original Boa project). By contributing, you agree to license your contributions under the same terms.
