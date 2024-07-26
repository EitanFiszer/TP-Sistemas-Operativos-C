int buscarDireccionFisicaEnTablaDePaginas(int pid, int pagina);

void* obtenerDatoMemoria(int direccion, int tamDato);

int cantidadMarcosLibres();

int buscarMarcoLibre();

void escribirMemoria(int direccion, void* dato, int tamDato);