#include "RawTableProcessing.hpp"
#include "RTPErrorHandler.hpp"

int check_data_type(std::string& data) {
	std::string::const_iterator it = data.begin();
	bool integer_type = false;
	bool decimal_point = false;
	bool string_type = true;
	while (it != data.end()) {
		if (*it == '.' && decimal_point) {
			// If decimal point appears more than once, then it is string type
			return 0;
		}
		if (*it == '.') {
			// If decimal point appears for the first time, then it may be a double
			decimal_point = true;
		}
		else {
			if (0 > * it || *it > 255) {
				// If char val is outside the ASCII range, then it is a string
				return 0;
			}
			else if (!std::isdigit(*it)) {
				// If non-digit value appears, then it is a string
				return 0;
			}
		}
		it++;
	}
	if (decimal_point) {
		// If a decimal point appeared only once, then it is a double
		return 2;
	}
	else {
		// If all chars are digits and no decimal point, then it is a integer
		return 1;
	}
}

Variant convert2integer(int idx, std::vector<std::vector<Variant>>& col) {
	Variant out;
	if (col[0][idx].get_type() == (int)DataType::STRING) {
		std::string raw_str = *(std::string*)col[0][idx].get_data();
		raw_str.erase(std::remove(raw_str.begin(), raw_str.end(), ','), raw_str.end());
		raw_str.erase(std::remove(raw_str.begin(), raw_str.end(), '"'), raw_str.end());
		raw_str.erase(std::remove(raw_str.begin(), raw_str.end(), ' '), raw_str.end());
		int ret_type = check_data_type(raw_str);
		if (ret_type != 1 && ret_type != 2) {
			RTP_Error ex(ErrorCode::INVALID_STR2INT);
			std::cerr << ex.what() << std::endl;
			throw ex;
		}
		else {
			out = std::stoi(raw_str);
		}
	}
	else if (col[0][idx].get_type() == (int)DataType::INTEGER) {
		out = col[0][idx];
	}
	return out;
}

Variant convert2double(int idx, std::vector<std::vector<Variant>>& col) {
	Variant out;
	if (col[0][idx].get_type() == (int)DataType::STRING) {
		std::string raw_str = *(std::string*)col[0][idx].get_data();
		raw_str.erase(std::remove(raw_str.begin(), raw_str.end(), '"'), raw_str.end());
		raw_str.erase(std::remove(raw_str.begin(), raw_str.end(), ' '), raw_str.end());
		std::replace(raw_str.begin(), raw_str.end(), ',', '.');
		int ret_type = check_data_type(raw_str);
		if (ret_type != 1 && ret_type != 2) {
			RTP_Error ex(ErrorCode::INVALID_STR2DBL);
			std::cerr << ex.what() << std::endl;
			throw ex;
		}
		else {
			out = std::stod(raw_str);
		}
	}
	else if (col[0][idx].get_type() == (int)DataType::DOUBLE) {
		out = col[0][idx];
	}
	return out;
}

void set_proper_col_names(Config& names_index, Table& raw_table) {
	for (int i = 0; i < raw_table.get_rows(); i++) {
		std::string itr_str = std::to_string(i);
		Variant value = names_index.get_value(itr_str)->get_num_str_data();
		std::string name_str = *(std::string*)value.get_data();
		Variant name_var(name_str);
		raw_table.update_cell_data(0, i, name_var);
	}
}


int process_pa_depto(Table*& raw_table, Config* main_config, Config* areas_config, Config* dept_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config = areas_config->get_value("PADepto")->get_list_data();
	raw_table = new Table(raw_tables_dir + *(std::string*)table_config[5].get_data(), ';');
	std::vector<Variant> table_col_config = main_config->get_value("PADepto_RTHdr")->get_list_data();
	if (raw_table->get_rows() != *(int*)table_config[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "PADepto -> Table rows: " << 
					 raw_table->get_rows() <<
				     " | Expected rows: " <<
					 *(int*)table_config[6].get_data() << 
					 "\n" << ex.what() <<
					 std::endl;
		throw ex;
		return -1;
	}
	// Set proper department names (STRING)
	set_proper_col_names(*dept_index, *raw_table);
	// Sanitize PCR tests data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[1].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize PR tests data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[2].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize AG tests data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[3].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Total tests data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[4].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	return 0;
}

int process_ca_depto(Table*& raw_table, Config* main_config, Config* areas_config, Config* dept_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config = areas_config->get_value("CADepto")->get_list_data();
	raw_table = new Table(raw_tables_dir + *(std::string*)table_config[5].get_data(), ';');
	std::vector<Variant> table_col_config = main_config->get_value("CADepto_RTHdr")->get_list_data();
	if (raw_table->get_rows() != *(int*)table_config[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CADepto -> Table rows: " <<
			raw_table->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	// Set proper department names (STRING)
	set_proper_col_names(*dept_index, *raw_table);
	// Sanitize PCR cases data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[1].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize PR cases data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[2].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize AG cases data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[3].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Total cases data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[4].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}	
	return 0;
}

int process_cp_edades(Table*& raw_table, Config* main_config, Config* areas_config, Config* age_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config = areas_config->get_value("CPEdades")->get_list_data();
	raw_table = new Table(raw_tables_dir + *(std::string*)table_config[5].get_data(), ';');
	std::vector<Variant> table_col_config = main_config->get_value("CPEdades_RTHdr")->get_list_data();
	if (raw_table->get_rows() != *(int*)table_config[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CPEdades -> Table rows: " <<
			raw_table->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	// Set proper department names (STRING)
	set_proper_col_names(*age_index, *raw_table);
	// Sanitize Total cases data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[1].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Tasa_Ataque data (DOUBLE)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[2].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2double);
	}
	// Sanitize Razon_Tasas data (DOUBLE)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[3].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2double);
	}
	return 0;
}

int process_ma_depto(Table*& raw_table, Config* main_config, Config* areas_config, Config* dept_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config = areas_config->get_value("MADepto")->get_list_data();
	raw_table = new Table(raw_tables_dir + *(std::string*)table_config[5].get_data(), ';');
	std::vector<Variant> table_col_config = main_config->get_value("MADepto_RTHdr")->get_list_data();
	if (raw_table->get_rows() != *(int*)table_config[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "MADepto -> Table rows: " <<
			raw_table->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	// Set proper department names (STRING)
	set_proper_col_names(*dept_index, *raw_table);
	// Sanitize Muertes_Total data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[1].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Muertes_Dia data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[2].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	return 0;
}

int process_ma_deptosm(Table*& raw_table, Config* main_config, Config* areas_config, Config* dept_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config = areas_config->get_value("MADepto")->get_list_data();
	raw_table = new Table(raw_tables_dir + *(std::string*)table_config[5].get_data(), ';');
	std::vector<Variant> table_col_config = main_config->get_value("MADeptoSM_RTHdr")->get_list_data();
	if (raw_table->get_rows() != *(int*)table_config[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "MADeptoSM -> Table rows: " <<
			raw_table->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	// Set proper department names (STRING)
	set_proper_col_names(*dept_index, *raw_table);
	// Sanitize Muertes_Conf data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[1].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Muertes_Sosp data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[2].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Total_Muertes_SisVig data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[3].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Muertes_SINADEF data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config[4].get_data() };
		raw_table->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	return 0;
}

int process_ca_distr_20(Table*& raw_table_p1, Table*& raw_table_p2, Config* main_config, Config* areas_config, Config* distr_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config_p1 = areas_config->get_value("CADistr20P1")->get_list_data();
	std::vector<Variant> table_config_p2 = areas_config->get_value("CADistr20P2")->get_list_data();
	raw_table_p1 = new Table(raw_tables_dir + *(std::string*)table_config_p1[5].get_data(), ';');
	raw_table_p2 = new Table(raw_tables_dir + *(std::string*)table_config_p2[5].get_data(), ';');
	std::vector<Variant> table_col_config_p1 = main_config->get_value("CADistr20P1_RTHdr")->get_list_data();
	std::vector<Variant> table_col_config_p2 = main_config->get_value("CADistr20P2_RTHdr")->get_list_data();
	if (raw_table_p1->get_rows() != *(int*)table_config_p1[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CADistr20P1 -> Table rows: " <<
			raw_table_p1->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p1[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	if (raw_table_p2->get_rows() != *(int*)table_config_p2[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CADistr20P2 -> Table rows: " <<
			raw_table_p2->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p2[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	std::vector<Variant> erase_fields = main_config->get_value("EraseFieldsCADistr20")->get_list_data();
	// Remove percentage column
	raw_table_p1->remove_column(*(std::string*)erase_fields[0].get_data());
	raw_table_p2->remove_column(*(std::string*)erase_fields[0].get_data());
	// Join part 1 and part 2
	raw_table_p1->join_tables(*raw_table_p2);	
	// Set proper district names (STRING)
	set_proper_col_names(*distr_index, *raw_table_p1);
	// Sanitize Casos_Totales data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[1].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Tasa_Ataque data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[2].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2double);
	}
	return 0;
}

int process_ca_distr_21(Table*& raw_table_p1, Table*& raw_table_p2, Config* main_config, Config* areas_config, Config* distr_index){
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config_p1 = areas_config->get_value("CADistr21P1")->get_list_data();
	std::vector<Variant> table_config_p2 = areas_config->get_value("CADistr21P2")->get_list_data();
	raw_table_p1 = new Table(raw_tables_dir + *(std::string*)table_config_p1[5].get_data(), ';');
	raw_table_p2 = new Table(raw_tables_dir + *(std::string*)table_config_p2[5].get_data(), ';');
	std::vector<Variant> table_col_config_p1 = main_config->get_value("CADistr21P1_RTHdr")->get_list_data();
	std::vector<Variant> table_col_config_p2 = main_config->get_value("CADistr21P2_RTHdr")->get_list_data();
	if (raw_table_p1->get_rows() != *(int*)table_config_p1[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CasosAcumuDistrito2021P1.csv -> Table rows: " <<
			raw_table_p1->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p1[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	if (raw_table_p2->get_rows() != *(int*)table_config_p2[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "CasosAcumuDistrito2021P2.csv -> Table rows: " <<
			raw_table_p2->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p2[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	std::vector<Variant> erase_fields = main_config->get_value("EraseFieldsCADistr21")->get_list_data();
	// Remove percentage column
	raw_table_p1->remove_column(*(std::string*)erase_fields[0].get_data());
	raw_table_p2->remove_column(*(std::string*)erase_fields[0].get_data());
	// Join part 1 and part 2
	raw_table_p1->join_tables(*raw_table_p2);
	// Set proper district names (STRING)
	set_proper_col_names(*distr_index, *raw_table_p1);
	// Sanitize Casos_Totales data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[1].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Tasa_Ataque data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[2].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2double);
	}
	return 0;
}

int process_ma_distr(Table*& raw_table_p1, Table*& raw_table_p2, Config* main_config, Config* areas_config, Config* distr_index) {
	std::string raw_tables_dir = *(std::string*)main_config->get_value("RawTablesDir")->get_num_str_data().get_data() + "/";
	std::vector<Variant> table_config_p1 = areas_config->get_value("MADistrP1")->get_list_data();
	std::vector<Variant> table_config_p2 = areas_config->get_value("MADistrP2")->get_list_data();
	raw_table_p1 = new Table(raw_tables_dir + *(std::string*)table_config_p1[5].get_data(), ';');
	raw_table_p2 = new Table(raw_tables_dir + *(std::string*)table_config_p2[5].get_data(), ';');
	std::vector<Variant> table_col_config_p1 = main_config->get_value("MADistrP1_RTHdr")->get_list_data();
	std::vector<Variant> table_col_config_p2 = main_config->get_value("MADistrP2_RTHdr")->get_list_data();
	if (raw_table_p1->get_rows() != *(int*)table_config_p1[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "MADistrP1 -> Table rows: " <<
			raw_table_p1->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p1[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	if (raw_table_p2->get_rows() != *(int*)table_config_p2[6].get_data()) {
		RTP_Error ex(ErrorCode::UNEQUAL_ROWS);
		std::cerr << "MADistrP2 -> Table rows: " <<
			raw_table_p2->get_rows() <<
			" | Expected rows: " <<
			*(int*)table_config_p2[6].get_data() <<
			"\n" << ex.what() <<
			std::endl;
		throw ex;
		return -1;
	}
	// Join part 1 and part 2
	raw_table_p1->join_tables(*raw_table_p2);
	// Set proper district names (STRING)
	set_proper_col_names(*distr_index, *raw_table_p1);
	// Sanitize Defunciones data (INTEGER)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[1].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2integer);
	}
	// Sanitize Tasa_Mortalidad data (DOUBLE)
	{
		std::vector<std::string> table_col_str = { *(std::string*)table_col_config_p1[2].get_data() };
		raw_table_p1->compute_update_column(table_col_str[0], table_col_str, convert2double);
	}
	
	return 0;
}