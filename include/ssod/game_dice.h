/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
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
