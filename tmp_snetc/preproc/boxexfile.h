/*******************************************************************************
 *
 * $Id: boxexfile.h 1723 2007-11-12 15:09:56Z cg $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   15.02.2007
 * -----
 *
 * Description:
 * ------------
 *
 * Box language code file handling.
 *
 *******************************************************************************/


#ifndef _SNETC_BOXEXFILE_H_
#define _SNETC_BOXEXFILE_H_

#include "types.h"

/*******************************************************************************
 *
 * Description: Open file.
 *
 * Parameters: - snetBase, snet file base name
 *             - boxBase, box language file base name
 *             - boxType, box language file type name
 *
 * Return: - TRUE, OK
 *         - FALSE, error
 *
 *******************************************************************************/
extern bool openFile(const char *snetBase, const char *boxBase, const char *boxType);

/*******************************************************************************
 *
 * Description: Close file.
 *
 * Parameters: -
 *
 * Return: -
 *
 *******************************************************************************/
extern void closeFile();

/*******************************************************************************
 *
 * Description: Write to file.
 *
 * Parameters: - data, data to write
 *
 * Return: -
 *
 *******************************************************************************/
extern void writeFile(const char *data);

#endif /* _SNETC_BOXEXFILE_H_ */
