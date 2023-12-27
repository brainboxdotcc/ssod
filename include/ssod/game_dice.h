#pragma once

/**
 * @brief Uniform distribution between first and last inclusive
 * 
 * @param first lowest number
 * @param last highest number
 * @return int random between first and last
 */
int random(int first, int last);

/**
 * @brief Uniform distribution D6
 * 
 * @return int between 1 and 6 inclusive
 */
int dice();

/**
 * @brief Uniform distribution D12
 * 
 * @return int between 1 and 12 inclusive
 */
int d12();

/**
 * @brief Uniform distribution 2D6
 * 
 * @return int between 2 and 12 inclusive
 */
int twodice();
