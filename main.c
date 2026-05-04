#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <time.h>

void registrar_jugador(PGconn *conn)
{
    char nombre[50], posicion[20];
    int partidos, puntos, id_equipo;
    float promedio;

    printf("\n---- EQUIPOS DISPONIBLES ----\n");
    PGresult *res_eq = PQexec(conn, "SELECT id_equipo, nombre_equipo FROM equipos");
    for (int i = 0; i < PQntuples(res_eq); i++)
    {
        printf("ID: %s | Equipo: %s\n", PQgetvalue(res_eq, i, 0), PQgetvalue(res_eq, i, 1));
    }

    PQclear(res_eq);

    printf("\n---- NUEVO FICHAJE ----\n\n");
    printf("Nombre de Jugador: ");
    scanf(" %[^\n]", nombre);
    printf("Posicion: ");
    scanf(" %[^\n]", posicion);
    printf("Partidos Jugados: ");
    scanf("%d", &partidos);
    printf("Puntos Totales: ");
    scanf("%d", &puntos);
    printf("ID de Equipo: ");
    scanf("%d", &id_equipo);

    promedio = (float)puntos / partidos;

    char query[512];
    sprintf(query, "INSERT INTO jugadores_liga(nombre_jugador, posicion, partidos_jugados, puntos_totales, id_equipo, promedio) VALUES ('%s', '%s', %d, %d, %d, %.2f)", nombre, posicion, partidos, puntos, id_equipo, promedio);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Erro al registrar: %s\n", PQerrorMessage(conn));
    }
    else
    {
        printf("\n%s se ha Registrado con Exito!\n", nombre);
    }

    PQclear(res);
}

void ver_ranking(PGconn *conn)
{
    printf("\n\t--- TOP RANKING DE LA LIGA ---\n\n");

    const char *sql = "SELECT j.nombre_jugador, e.nombre_equipo, j.promedio "
                      "FROM jugadores_liga j "
                      "INNER JOIN equipos e ON j.id_equipo = e.id_equipo "
                      "ORDER BY j.promedio DESC;";

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error al leer: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("\n%-4s | %-20s | %-15s | %-10s", "POS", "JUGADOR", "EQUIPO", "PROMEDIO");
    printf("\n------------------------------------------------------------------------\n");
    for (int i = 0; i < PQntuples(res); i++)
    {
        printf("%-4d | %-20s | %-15s | %.2f\n", i + 1, PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), atof(PQgetvalue(res, i, 2)));
    }

    PQclear(res);
}

void actualizar_partido(PGconn *conn)
{
    int id_jugador, puntos_nuevos;
    char s_id[10], s_puntos[10];

    printf("\n---- JUGADORES REGISTRADOS ----\n\n");
    PGresult *res_lista = PQexec(conn, "SELECT id_jugador, nombre, puntos_totales FROM jugadores_liga ORDER BY id_jugador");
    for (int i = 0; i < PQntuples(res_lista); i++)
    {
        printf("ID: %s | Nombre: %-15s | Puntos Actuales: %s\n", PQgetvalue(res_lista, i, 0), PQgetvalue(res_lista, i, 1), PQgetvalue(res_lista, i, 2));
    }

    PQclear(res_lista);

    printf("\nID de el Jugador a Actualizar: ");
    scanf("%d", &id_jugador);
    printf("Puntos anotados en este partido: ");
    scanf("%d", &puntos_nuevos);

    // Conviertes los enteros a cadenas para PQexecParams
    snprintf(s_id, sizeof(s_id), "%d", id_jugador);
    snprintf(s_puntos, sizeof(s_puntos), "%d", puntos_nuevos);

    const char *paramValues[2] = {s_puntos, s_id};

    // Consulta parametrizada segura ($1 = puntos, $2 = id)
    const char *sql = "UPDATE jugadores_liga SET puntos_totales = puntos_totales + $1, partidos_jugados = partidos_jugados + 1, promedio = (puntos_totales + $1)::float / (partidos_jugados + 1) -- <--- Division real WHERE id_jugador = $2";

    PGresult *res = PQexecParams(conn, sql, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Error al actualizar: %s\n", PQerrorMessage(conn));
    }
    else
    {
        printf("\nEstadisticas actualizadas exitosamente!\n");
    }

    PQclear(res);
}

void ranking_equipos(PGconn *conn)
{
    printf("\n\t--- RANKING GENERAL POR EQUIPOS ---\n");

    const char *sql = "SELECT e.nombre_equipo, COUNT(j.id_jugador), SUM(j.puntos_totales) as total "
                      "FROM equipos e "
                      "LEFT JOIN jugadores_liga j ON e.id_equipo = j.id_equipo "
                      "GROUP BY e.nombre_equipo "
                      "ORDER BY total DESC NULLS LAST";
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error de lectura: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("\n%-4s | %-20s | %-10s | %-10s\n", "POS", "EQUIPO", "JUGADORES", "PTS TOTALES");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < PQntuples(res); i++)
    {
        // Si SUM devuelve nulo si no hay puntos, ponemos 0
        char *pts_str = PQgetisnull(res, i, 2) ? "0" : PQgetvalue(res, i, 2);
        printf("%-4d | %-20s | %-10s | %-10s\n", i + 1, PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), pts_str);
    }
    PQclear(res);
}

void retirar_jugador(PGconn *conn)
{
    int id_jugador;
    char confirmacion, s_id[10];

    printf("\n\t---- RETIRO DE JUGADOR ----\n");
    PGresult *res_lista = PQexec(conn, "SELECT id_jugador, nombre_jugador FROM jugadores_liga ORDER BY id_jugador");
    for (int i = 0; i < PQntuples(res_lista); i++)
    {
        printf("ID: %s | Nombre: %s\n", PQgetvalue(res_lista, i, 0), PQgetvalue(res_lista, i, 1));
    }

    PQclear(res_lista);

    printf("\nID de Jugador a Retirarse: ");
    scanf("%d", &id_jugador);

    snprintf(s_id, sizeof(s_id), "%d", id_jugador);
    const char *paramValue[1] = {s_id};

    const char *sql = "SELECT nombre_jugador FROM jugadores_liga WHERE id_jugador = $1";
    PGresult *res = PQexecParams(conn, sql, 1, NULL, paramValue, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        fprintf(stderr, "Jugador no Encontrado: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    char nombre[50];
    strcpy(nombre, PQgetvalue(res, 0, 0));
    PQclear(res);

    printf("\nQueres dar de baja a %s? (s/n): ", nombre);
    scanf(" %c", &confirmacion);

    if (confirmacion == 's' || confirmacion == 'S')
    {
        const char *sql_delete = "DELETE FROM jugadores_liga WHERE id_jugador = $1";
        PGresult *res_delete = PQexecParams(conn, sql_delete, 1, NULL, paramValue, NULL, NULL, 0);

        if (PQresultStatus(res_delete) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "Error al eliminar: %s\n", PQerrorMessage(conn));
        }
        else
        {
            printf("\nExito: %s ha sido retirado de la liga.\n", nombre);
        }

        PQclear(res_delete);
    }
    else
    {
        printf("\nOperacion cancelada. %s sigue en el equipo.\n", nombre);
    }
}

void buscar_jugador(PGconn *conn)
{
    char busqueda[50];
    char filtro[55];

    printf("\n--- BUSCADOR DE JUGADOR ---");
    printf("\nIngresa el nombre o parte del nombre: ");
    scanf(" %[^\n]", busqueda);

    snprintf(filtro, sizeof(filtro), "%%%s%%", busqueda);

    const char *paramValues[1] = {filtro};

    const char *sql = "SELECT j.nombre_jugador, e.nombre_equipo, j.promedio FROM jugadores_liga j "
                      "INNER JOIN equipos e ON j.id_equipo = e.id_equipo "
                      "WHERE j.nombre_jugador ILIKE $1 " // busca el nombre de jugador sea minuscula y mayusculas
                      "ORDER BY j.nombre_jugador ASC";

    PGresult *res = PQexecParams(conn, sql, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error en la busqueda: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    int filas = PQntuples(res);
    if (filas == 0)
    {
        printf("\nNo se encontro ningun jugador que coincida con '%s'.\n", busqueda);
    }
    else
    {
        printf("\nResultados encontrados (%d):\n", filas);
        for (int i = 0; i < filas; i++)
        {
            printf("- %s (Equipo: %s)\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));
        }
    }

    PQclear(res);
}

void transferir_jugador(PGconn *conn)
{
    int id_jugador, id_equipo_nuevo;
    char s_id_j[10], s_id_e[10];

    printf("\n--- MERCADO DE TRANSFERENCIA ---\n");
    printf("ID del Jugador a transferir: ");
    scanf("%d", &id_jugador);
    printf("ID del Nuevo Equipo: ");
    scanf("%d", &id_equipo_nuevo);

    snprintf(s_id_j, sizeof(s_id_j), "%d", id_jugador);
    snprintf(s_id_e, sizeof(s_id_e), "%d", id_equipo_nuevo);
    const char *params[2] = {s_id_e, s_id_j};

    // Checkeo de si Existe el Equipo o no
    const char *sql_check = "SELECT nombre_equipo FROM equipos WHERE id_equipo = $1";
    const char *checkParam[1] = {s_id_e};
    PGresult *res_check = PQexecParams(conn, sql_check, 1, NULL, checkParam, NULL, NULL, 0);

    if (PQntuples(res_check) == 0)
    {
        printf("\nError: El equipo con ID %d no existe.\n", id_equipo_nuevo);
        PQclear(res_check);
        return;
    }

    char nombre_eq[50];
    strcpy(nombre_eq, PQgetvalue(res_check, 0, 0));
    PQclear(res_check);

    // El Traspaso de Jugador
    const char *sql_update = "UPDATE jugadores_liga SET id_equipo = $1 WHERE id_jugador = $2";
    PGresult *res_upd = PQexecParams(conn, sql_update, 2, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res_upd) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "\nFallo el traspaso: %s\n", PQerrorMessage(conn));
    }
    else
    {
        // PQcmdTuples dice cuantas filas se afectaron
        if (atoi(PQcmdTuples(res_upd)) == 0)
        {
            printf("\nNo se encontro al jugador con ID %d.\n", id_jugador);
        }
        else
        {
            printf("\nTraspaso exitoso a %s! Bienvenido Kplu!\n", nombre_eq);
        }
    }

    PQclear(res_upd);
}

void registrar_partido(PGconn *conn)
{
    int local, visitante, pts_l, pts_v;
    char s_l[10], s_v[10], s_pl[10], s_pv[10];

    printf("\n--- REGISTRAR PARTIDO TORNEO APERTURA) ---\n");
    printf("ID Equipo Local: ");
    scanf("%d", &local);
    printf("ID Equipo Visitante: ");
    scanf("%d", &visitante);
    printf("Puntos Local: ");
    scanf("%d", &pts_l);
    printf("Puntos Visitante: ");
    scanf("%d", &pts_v);

    snprintf(s_l, 10, "%d", local);
    snprintf(s_v, 10, "%d", visitante);
    snprintf(s_pl, 10, "%d", pts_l);
    snprintf(s_pv, 10, "%d", pts_v);

    const char *params[4] = {s_l, s_v, s_pl, s_pv};
    const char *sql = "INSERT INTO partidos (id_equipo_local, id_equipo_visitante, puntos_local, puntos_visitante) VALUES ($1, $2, $3, $4)";

    PGresult *res = PQexecParams(conn, sql, 4, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Error al registrar partido: %s\n", PQerrorMessage(conn));
    }
    else
    {
        printf("\nResultado guardado con exito!\n");
        if (pts_l > pts_v)
        {
            printf("Ganador: Local (+2 pts)\n");
        }
        else
        {
            printf("Ganador: Visitante (+2 pts)\n");
        }
    }
    PQclear(res);
}

void ver_tabla_posiciones(PGconn *conn)
{
    const char *sql =
        "SELECT e.nombre_equipo, "
        "COUNT(p.id_partido) as PJ, "
        "SUM(CASE WHEN (p.id_equipo_local = e.id_equipo AND p.puntos_local > p.puntos_visitante) OR "
        "             (p.id_equipo_visitante = e.id_equipo AND p.puntos_visitante > p.puntos_local) "
        "        THEN 1 ELSE 0 END) as PG, "
        "SUM(CASE WHEN (p.id_equipo_local = e.id_equipo AND p.puntos_local < p.puntos_visitante) OR "
        "             (p.id_equipo_visitante = e.id_equipo AND p.puntos_visitante < p.puntos_local) "
        "        THEN 1 ELSE 0 END) as PP, "
        "SUM(CASE "
        "    WHEN p.id_partido IS NULL THEN 0 "
        "    WHEN (p.id_equipo_local = e.id_equipo AND p.puntos_local > p.puntos_visitante) OR "
        "         (p.id_equipo_visitante = e.id_equipo AND p.puntos_visitante > p.puntos_local) "
        "    THEN 2 "
        "    ELSE 1 "
        "END) as PTS "
        "FROM equipos e "
        "LEFT JOIN partidos p ON e.id_equipo = p.id_equipo_local OR e.id_equipo = p.id_equipo_visitante "
        "GROUP BY e.nombre_equipo "
        "ORDER BY PTS DESC, PG DESC;"; //valgame dios

    PGresult *res = PQexec(conn, sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error al obtener posiciones: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("\n==========================================================\n");
    printf("             TABLA DE POSICIONES - APERTURA               \n");
    printf("==========================================================\n");
    printf("%-20s | %-3s | %-3s | %-3s | %-4s\n", "EQUIPO", "PJ", "PG", "PP", "PTS");
    printf("----------------------------------------------------------\n");

    int filas = PQntuples(res);
    for (int i = 0; i < filas; i++)
    {
        printf("%-20s | %-3s | %-3s | %-3s | %-4s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), PQgetvalue(res, i, 2), PQgetvalue(res, i, 3), PQgetvalue(res, i, 4));
    }
    printf("==========================================================\n");

    PQclear(res);
}

void simular_partido_random(PGconn *conn)
{
    srand(time(NULL));

    PGresult *res_eq = PQexec(conn, "SELECT id_equipo, nombre_equipo FROM equipos");
    int num_equipos = PQntuples(res_eq);

    if (num_equipos < 2)
    {
        printf("\nNecesitas al menos 2 equipos para simular un partido.\n");
        PQclear(res_eq);
        return;
    }

    // Elijo 2 equipos al azar
    int idx1 = rand() % num_equipos;
    int idx2;
    do
    {
        idx2 = rand() % num_equipos;
    } while (idx1 == idx2);

    int id_eq1 = atoi(PQgetvalue(res_eq, idx1, 0));
    int id_eq2 = atoi(PQgetvalue(res_eq, idx2, 0));
    char *nom_eq1 = PQgetvalue(res_eq, idx1, 1);
    char *nom_eq2 = PQgetvalue(res_eq, idx2, 1);

    int pts1 = (rand() % 51) + 60;
    int pts2 = (rand() % 51) + 60;

    printf("\n--- SIMULACION DE PARTIDO ---\n");
    printf("%s %d - %d %s\n", nom_eq1, pts1, pts2, nom_eq2);

    // INSERTAR EL PARTIDO EN LA TABLA
    char s_id1[10], s_id2[10], s_p1[10], s_p2[10];
    sprintf(s_id1, "%d", id_eq1);
    sprintf(s_id2, "%d", id_eq2);
    sprintf(s_p1, "%d", pts1);
    sprintf(s_p2, "%d", pts2);

    const char *params[4] = {s_id1, s_id2, s_p1, s_p2};
    PQexecParams(conn, "INSERT INTO partidos (id_equipo_local, id_equipo_visitante, puntos_local, puntos_visitante) VALUES ($1, $2, $3, $4)", 4, NULL, params, NULL, NULL, 0);

    // Reparticion de Puntos a Jugadores
    // Buscqueda de cada jugador de cada de cada equipo y se le da x puntos
    int ids_equipos[2] = {id_eq1, id_eq2};
    int puntos_equipos[2] = {pts1, pts2};

    for (int i = 0; i < 2; i++)
    {
        char s_eq[10];
        sprintf(s_eq, "%d", ids_equipos[i]);
        const char *p_eq[1] = {s_eq};
        PGresult *res_jug = PQexecParams(conn, "SELECT id_jugador FROM jugadores_liga WHERE id_equipo = $1", 1, NULL, p_eq, NULL, NULL, 0);

        int n_jug = PQntuples(res_jug);
        if (n_jug > 0)
        {
            // Divide los puntos del equipo equitativamente entre sus jugadores
            int pts_por_jugador = puntos_equipos[i] / n_jug;
            char s_pts[10];
            sprintf(s_pts, "%d", pts_por_jugador);

            for (int j = 0; j < n_jug; j++)
            {
                const char *upd_params[2] = {s_pts, PQgetvalue(res_jug, j, 0)};
                PQexecParams(conn, "UPDATE jugadores_liga SET promedio = promedio + $1 WHERE id_jugador = $2", 2, NULL, upd_params, NULL, NULL, 0);
            }
        }

        PQclear(res_jug);
    }

    printf("\nPartido simulado\n");

    PQclear(res_eq);
}

void lista_buena_fe(PGconn *conn)
{
    int id_equipo;
    char s_id[10];

    printf("\n--- LISTA DE BUENA FE POR EQUIPO ---\n");

    PGresult *res_equipos = PQexec(conn, "SELECT id_equipo, nombre_equipo FROM equipos ORDER BY id_equipo");
    printf("\nEquipos registrados:\n");
    for (int i = 0; i < PQntuples(res_equipos); i++)
    {
        printf("%s. %s\n", PQgetvalue(res_equipos, i, 0), PQgetvalue(res_equipos, i, 1));
    }

    PQclear(res_equipos);

    // Pedir el ID del equipo
    printf("\nIngrese el ID del equipo para ver su lista: ");
    scanf("%d", &id_equipo);

    snprintf(s_id, sizeof(s_id), "%d", id_equipo);
    const char *params[1] = {s_id};

    // Consulta SQL: Traemos nombre del equipo y sus jugadores
    // JOIN para mostrar el nombre del equipo arriba como cabecera
    const char *sql =
        "SELECT e.nombre_equipo, j.nombre_jugador, j.id_jugador, j.promedio "
        "FROM equipos e "
        "LEFT JOIN jugadores_liga j ON e.id_equipo = j.id_equipo "
        "WHERE e.id_equipo = $1 "
        "ORDER BY j.nombre_jugador ASC";

    PGresult *res = PQexecParams(conn, sql, 1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        printf("\n[!] El equipo no existe o no tiene jugadores cargados.\n");
        PQclear(res);
        return;
    }

    // Formateo de la lista
    printf("\n========================================\n");
    printf(" EQUIPO: %s\n", PQgetvalue(res, 0, 0));
    printf("========================================\n");
    printf("%-5s | %-20s | %-8s\n", "ID", "NOMBRE JUGADOR", "PUNTOS");
    printf("----------------------------------------\n");

    for (int i = 0; i < PQntuples(res); i++)
    {
        // Si el equipo existe pero no tiene jugadores, el JOIN trae NULL en los campos de jugador
        if (PQgetisnull(res, i, 1))
        {
            printf("Sin jugadores registrados todavia.\n");
        }
        else
        {
            printf("%-5s | %-20s | %-8s\n",
                   PQgetvalue(res, i, 2),
                   PQgetvalue(res, i, 1),
                   PQgetvalue(res, i, 3));
        }
    }
    printf("========================================\n");

    PQclear(res);
}

void exportar_lista_txt(PGconn *conn)
{
    int id_equipo;
    char s_id[10];

    // Elegir el equipo
    PGresult *res_eqs = PQexec(conn, "SELECT id_equipo, nombre_equipo FROM equipos ORDER BY id_equipo");
    printf("\nEquipos disponibles:\n");
    for (int i = 0; i < PQntuples(res_eqs); i++)
    {
        printf("%s. %s\n", PQgetvalue(res_eqs, i, 0), PQgetvalue(res_eqs, i, 1));
    }
    printf("\nIngrese ID del equipo para exportar su lista: ");
    scanf("%d", &id_equipo);
    PQclear(res_eqs);

    snprintf(s_id, sizeof(s_id), "%d", id_equipo);
    const char *params[1] = {s_id};

    // Traer los datos de la BD
    const char *sql =
        "SELECT e.nombre_equipo, j.nombre_jugador, j.posicion, j.promedio "
        "FROM equipos e "
        "JOIN jugadores_liga j ON e.id_equipo = j.id_equipo "
        "WHERE e.id_equipo = $1 "
        "ORDER BY j.nombre_jugador ASC";

    PGresult *res = PQexecParams(conn, sql, 1, NULL, params, NULL, NULL, 0);

    if (PQntuples(res) == 0)
    {
        printf("\nEl equipo no tiene jugadores o no existe.\n");
        PQclear(res);
        return;
    }

    char nombre_archivo[100];
    sprintf(nombre_archivo, "Lista_%s.txt", PQgetvalue(res, 0, 0));

    FILE *archivo = fopen(nombre_archivo, "w");

    if (archivo == NULL)
    {
        printf("Error al crear el archivo.\n");
        PQclear(res);
        return;
    }

    fprintf(archivo, "====================================================\n");
    fprintf(archivo, "        LISTA DE BUENA FE - TORNEO APERTURA         \n");
    fprintf(archivo, "====================================================\n");
    fprintf(archivo, "EQUIPO: %s\n", PQgetvalue(res, 0, 0));
    fprintf(archivo, "FECHA DE EMISION: 04/04/2026\n"); // Actual
    fprintf(archivo, "----------------------------------------------------\n");
    fprintf(archivo, "%-25s | %-15s | %-5s\n", "NOMBRE JUGADOR", "POSICION", "PTS");
    fprintf(archivo, "----------------------------------------------------\n");

    for (int i = 0; i < PQntuples(res); i++)
    {
        fprintf(archivo, "%-25s | %-15s | %-5s\n", PQgetvalue(res, i, 1), PQgetvalue(res, i, 2), PQgetvalue(res, i, 3));
    }

    fprintf(archivo, "----------------------------------------------------\n");
    fprintf(archivo, "\n\nFirma del Delegado: __________________________\n");

    fclose(archivo);
    PQclear(res);

    printf("\nSe ha generado el archivo: %s\n", nombre_archivo);
}

int main()
{
    const char *conneinfo = "host=localhost port=5432 dbname=liga_univ_bd user=postgres password=1234";
    PGconn *conn = PQconnectdb(conneinfo);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Error de conexion: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    int opcion;

    do
    {
        printf("\n\t=================================");
        printf("\n\t    SISTEMA DE LIGA PRINCIPAL    ");
        printf("\n\t=================================");
        printf("\n\n1. Fichar nuevo jugador");
        printf("\n2. Ver Ranking Individual");
        printf("\n3. Actualizar Partido (Sumar Puntos)");
        printf("\n4. Ver Ranking por Equipos");
        printf("\n5. Retirar Jugador");
        printf("\n6. Buscar Jugador");
        printf("\n7. Trasferecia de Jugador");
        printf("\n8. Registrar Partido");
        printf("\n9. Ver Tabla de Posiciones");
        printf("\n10. Simular Partido");
        printf("\n11. Lista de Buena FE");
        printf("\n12. Exportar Lista de Buena FE");
        printf("\n13. Salir");
        printf("\n\nOpcion: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            registrar_jugador(conn);
            break;
        case 2:
            ver_ranking(conn);
            break;
        case 3:
            actualizar_partido(conn);
            break;
        case 4:
            ranking_equipos(conn);
            break;
        case 5:
            retirar_jugador(conn);
            break;
        case 6:
            buscar_jugador(conn);
            break;
        case 7:
            transferir_jugador(conn);
            break;
        case 8:
            registrar_partido(conn);
            break;
        case 9:
            ver_tabla_posiciones(conn);
            break;
        case 10:
            simular_partido_random(conn);
            break;
        case 11:
            lista_buena_fe(conn);
            break;
        case 12:
            exportar_lista_txt(conn);
            break;
        case 13:
            printf("\n-- Fin del Programa --\n");
            break;
        default:
            printf("\nOpcion no valida.\n");
        }
    } while (opcion != 13);

    PQfinish(conn);
    return 0;
}