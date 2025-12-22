#ifndef PEACHDB_H
#define PEACHDB_H

/**
 * Initialize the PeachDB service.
 * Check folder 'peachdata', 'peachdata/collections', 'peachdata/index.pdb'.
 * If the folder or file does not exist, create it.
 * @return 0 on success, -1 on failure.
 */
int Peach_initPeachDb();

#endif // PEACHDB_H
