/*
 * Copyright (c) 2008-2011 Zhang Ming (M. Zhang), zmjerry@163.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 or any later version.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details. A copy of the GNU General Public License is available at:
 * http://www.fsf.org/licensing/licenses
 */


/*****************************************************************************
 *                               steepdesc.h
 *
 * Steepest descent optimal method( gradient algorithm ).
 *
 * This class is designed for finding the minimum value of objective function
 * in one dimension or multidimension. Inexact line search algorithm is used
 * to get the step size in each iteration.
 *
 * Zhang Ming, 2010-03, Xi'an Jiaotong University.
 *****************************************************************************/


#ifndef STEEPDESC_H
#define STEEPDESC_H


#include "linesearch.h"


namespace splab
{

    template <typename Dtype, typename Ftype>
    class SteepDesc : public LineSearch<Dtype, Ftype>
    {

    public:

        SteepDesc();
        ~SteepDesc();

        void optimize( Ftype &func, Vector<Dtype> &x0,
                       Dtype tol=Dtype(1.0e-6), int maxItr=100 );

        Vector<Dtype> getOptValue() const;
        Vector<Dtype> getGradNorm() const;
        Dtype getFuncMin() const;
        int getItrNum() const;

    private:

        // iteration number
        int itrNum;

        // minimum value of objective function
        Dtype fMin;

        // optimal solution
        Vector<Dtype> xOpt;

        // gradient norm for each iteration
        Vector<Dtype> gradNorm;

    };
    // class SteepDesc


    #include "steepdesc-impl.h"

}
// namespace splab


#endif
// STEEPDESC_H
