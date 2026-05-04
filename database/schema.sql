CREATE TABLE equipos (
    id_equipo SERIAL PRIMARY KEY,
    nombre_equipo VARCHAR(50) NOT NULL
);


CREATE TABLE jugadores_liga (
    id_jugador SERIAL PRIMARY KEY,
    nombre_jugador VARCHAR(100) NOT NULL,
    posicion VARCHAR(20),
    partidos_jugados INTEGER DEFAULT 0,
    puntos_totales INTEGER DEFAULT 0,
    id_equipo INTEGER REFERENCES equipos(id_equipo),
    promedio NUMERIC(4,2) DEFAULT 0.00 
);

CREATE TABLE partidos (
    id_partido SERIAL PRIMARY KEY,
    id_equipo_local INT REFERENCES equipos(id_equipo),
    id_equipo_visitante INT REFERENCES equipos(id_equipo),
    puntos_local INT,
    puntos_visitante INT,
    nombre_torneo VARCHAR(20) DEFAULT 'Apertura 2026',
    fecha_partido TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


SELECT e.nombre_equipo, COUNT(p.id_partido) as jugados
FROM equipos e
LEFT JOIN partidos p ON (e.id_equipo = p.id_equipo_local OR e.id_equipo = p.id_equipo_visitante)
WHERE p.nombre_torneo = 'Apertura 2026'
GROUP BY e.nombre_equipo;

INSERT INTO equipos (nombre_equipo) VALUES
('Ingenieria'),
('Economia'),
('Medicina');

SELECT * FROM equipos
SELECT * FROM jugadores_liga
SELECT * FROM partidos
DROP TABLE IF EXISTS jugadores_liga CASCADE;
DROP TABLE IF EXISTS equipos CASCADE;

TRUNCATE TABLE partidos RESTART IDENTITY;
UPDATE jugadores_liga SET promedio = 0;