#include "SaneUtils.h"

SANE_Int GetSaneInt(SANE_Handle device, const char *name)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;
	
	void *optval;
	int option_num;

	SANE_Int val = 0;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return val;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				optval = malloc(opt->size);
				status = sane_control_option (device, option_num, SANE_ACTION_GET_VALUE, optval, NULL);
				if (status == SANE_STATUS_GOOD) {
					if (opt->type == SANE_TYPE_FIXED) {
						val = (SANE_Int)(SANE_UNFIX(*(SANE_Word*) optval));
					} else {
						val = *(SANE_Word*) optval;
					}
				}
				free(optval);
			}
	}
	return val;
}


float GetSaneFloat(SANE_Handle device, const char *name)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;
	
	void *optval;
	int option_num;

	float val = 0;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return val;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				optval = malloc(opt->size);
				status = sane_control_option (device, option_num, SANE_ACTION_GET_VALUE, optval, NULL);
				if (status == SANE_STATUS_GOOD) {
					if (opt->type == SANE_TYPE_FIXED) {
						val = SANE_UNFIX(*(SANE_Word*) optval);
					} else {
						val = *(SANE_Word*) optval;
					}
				}
				free(optval);
			}
	}
	return val;
}


float GetSaneMaxFloat(SANE_Handle device, const char *name)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;
	
	void *optval;
	int option_num;

	float val = 0;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return val;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				optval = malloc(opt->size);
				status = sane_control_option (device, option_num, SANE_ACTION_GET_VALUE, optval, NULL);
				if (status == SANE_STATUS_GOOD) {
					if (opt->type == SANE_TYPE_FIXED) {
						val = SANE_UNFIX(opt->constraint.range->max);						
					} else {
						val = opt->constraint.range->max;
					}
				}
				free(optval);
			}
	}
	return val;
}


float GetSaneMinFloat(SANE_Handle device, const char *name)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;

	void *optval;
	int option_num;

	float val = 0;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return val;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				optval = malloc(opt->size);
				status = sane_control_option (device, option_num, SANE_ACTION_GET_VALUE, optval, NULL);
				if (status == SANE_STATUS_GOOD) {
					if (opt->type == SANE_TYPE_FIXED) {
						val = SANE_UNFIX(opt->constraint.range->min);
					} else {
						val = opt->constraint.range->min;
					}
				}
				free(optval);
			}
	}
	return val;
}


void SetSaneInt(SANE_Handle device, const char *name, SANE_Int val)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;
	SANE_Int 	info;
	SANE_Word	word_value = val;

	void *optval = &word_value;
	int option_num;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				status = sane_control_option (device, option_num, SANE_ACTION_SET_VALUE, optval, &info);
			}
	}
}



void SetSaneFloat(SANE_Handle device, const char *name, float val)
{
	const SANE_Option_Descriptor *opt;
	SANE_Int num_dev_options;
	SANE_Status status;
	SANE_Int 	info;
	SANE_Word	word_value = SANE_FIX(val);

	void *optval = &word_value;
	int option_num;

	status = sane_control_option (device, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
	if(status != SANE_STATUS_GOOD)
		return;

	for (option_num = 0; option_num < num_dev_options; option_num++) {
		opt = sane_get_option_descriptor (device, option_num);
		if (opt)
			if (opt->name && strcmp(opt->name, name) == 0) {
				status = sane_control_option (device, option_num, SANE_ACTION_SET_VALUE, optval, &info);
			}
	}
}
