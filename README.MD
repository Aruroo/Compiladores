# My compiler

This repository aims to store all code needed for creating a Compiler
using tools such as Flex and Bison. 
Therefore, all the code is implemented in C programming language.

## Development

### Branching Strategy

This project follows a simple and effective branching model:

- **`main`** - The stable branch. Contains production-ready code that compiles and passes all tests.
  
- **`develop`** - The integration branch. Active development happens here. Features are merged into `develop` first for testing before being merged to `main`.

- **Feature branches** - For new features or experiments:
  - Naming: `feature/feature-name` (e.g., `feature/add-loops`, `feature/type-system`)
  - Branch from: `develop`
  - Merge back to: `develop`
  
- **Bugfix branches** - For fixing bugs:
  - Naming: `bugfix/bug-description` (e.g., `bugfix/parser-error`)
  - Branch from: `develop` (or `main` for hotfixes)
  - Merge back to: `develop` (and `main` if hotfix)

**Workflow Example:**
```bash
# Create a new feature branch
git checkout develop
git checkout -b feature/add-functions

# Work on your feature, commit changes
git add .
git commit -m "Add function declaration parsing"

# When ready, merge back to develop
git checkout develop
git merge feature/add-functions
git branch -d feature/add-functions

# Periodically merge develop into main when stable
git checkout main
git merge develop
```


## Contributing

1. Fork the repository
2. Create your feature branch from `develop`
3. Commit your changes
4. Push to your branch
5. Create a Pull Request to `develop`
