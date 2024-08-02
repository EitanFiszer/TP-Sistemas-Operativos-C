int buscarDireccionFisicaEnTablaDePaginas(int pid, int pagina);

void* obtenerDatoMemoria(int pid, int direccion, int tamDato);

int cantidadMarcosLibres();

int buscarMarcoLibre();

void escribirMemoria(int pid, int direccion, void* dato, int tamDato);