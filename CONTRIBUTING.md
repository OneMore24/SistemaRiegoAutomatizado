# Guía de Contribución — AyDA 2026-I

## Flujo de trabajo con Git

### Ramas principales
- `main` — versión estable entregable
- `develop` — integración de cambios
- `feature/nombre-feature` — desarrollo de características

### Cómo trabajar

```bash
# 1. Clonar el repositorio
git clone https://github.com/TU_USUARIO/riego-automatizado-ayda.git
cd riego-automatizado-ayda

# 2. Crear rama para tu trabajo
git checkout -b feature/mi-algoritmo

# 3. Hacer cambios, compilar y ejecutar tests
g++ -std=c++17 -O2 -o test_small tests/test_small.cpp
./test_small

# 4. Commit con mensaje descriptivo
git add .
git commit -m "feat: implementar algoritmo greedy con EMA filter"

# 5. Push y Pull Request a develop
git push origin feature/mi-algoritmo
```

### Convención de commits

```
feat:    nueva funcionalidad
fix:     corrección de bug
docs:    documentación
test:    agregar/modificar tests
refactor: refactorización sin cambio de lógica
perf:    mejora de rendimiento
```

### Antes de cada commit

- [ ] El código compila sin errores
- [ ] `test_small` pasa sin fallos
- [ ] El código sigue el estilo del proyecto (nombres en inglés, comentarios en español)