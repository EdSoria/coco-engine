# Pendientes

## API y producto

- [x] Implementar `GET /api/puzzle/goal`.
- [x] Implementar `GET /api/puzzle/goal-types`.
- [x] Implementar `POST /api/puzzle/new`.
- [x] Implementar `POST /api/puzzle/move`.
- [x] Implementar `POST /api/puzzle/validate`.
- [x] Implementar `POST /api/puzzle/solve`.
- [x] Añadir pruebas HTTP automatizadas para todas las rutas y errores.
- [x] Completar el `Dockerfile` para compilar y ejecutar el servidor.
- [x] Alinear `README.md` y `docs/API.md` con el comportamiento real; `contrato.md` es documentación personal.

## Robustez y rendimiento

- [ ] Definir y documentar el contrato de `NodeLimitReached` frente a `timeout`.
- [ ] Añadir un límite temporal configurable al solver.
- [ ] Decidir si `make_random_board` debe devolver fallback o error al agotar intentos.
- [ ] Añadir cobertura para objetivos personalizados, movimientos inválidos y tableros 4x4/5x5.
- [x] Regenerar `build/` usando un único generador de CMake.
