/*---------------------------------------------------------------------------*\
    CFDEMcoupling academic - Open Source CFD-DEM coupling

    Contributing authors:
    Thomas Lichtenegger
    Copyright (C) 2015- Johannes Kepler University, Linz
-------------------------------------------------------------------------------
License
    This file is part of CFDEMcoupling academic.

    CFDEMcoupling academic is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CFDEMcoupling academic is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with CFDEMcoupling academic.  If not, see <http://www.gnu.org/licenses/>.
\*---------------------------------------------------------------------------*/

#include "error.H"
#include "diffNorm.H"
#include "recModel.H"
#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(diffNorm, 0);

addToRunTimeSelectionTable
(
    recNorm,
    diffNorm,
    dictionary
);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
diffNorm::diffNorm
(
    const dictionary& dict,
    recBase& base,
    word type
)
:
    recNorm(dict, base),
    propsDict_(dict.subDict(type + "Props")),
    normConstant_(propsDict_.lookupOrDefault<scalar>("normConstant",-1.0)),
    fieldType_(propsDict_.lookup("fieldType")),
    fieldName_(propsDict_.lookup("fieldName")),
    integrationDomainName_(propsDict_.lookupOrDefault<word>("integrationDomainName","all")),
    integrationDomain_(),
    existIntegrationDomain_(false)
{
    if(integrationDomainName_ != "all")
    {
       integrationDomain_.set(new cellSet(base_.mesh(),integrationDomainName_));
       existIntegrationDomain_ = true;
       Info << "diffNorm: restricting comparison to cellSet " << integrationDomain_().name() << endl;
    }
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

diffNorm::~diffNorm()
{}

// * * * * * * * * * * * * * protected Member Functions  * * * * * * * * * * * * //

void diffNorm::computeRecMatrix()
{
    Info << nl << type() << ": computing recurrence matrix\n" << endl;

    label totNumRecSteps = base_.recM().numRecFields();
    SymmetricSquareMatrix<scalar>& recurrenceMatrix( base_.recM().recurrenceMatrix() );

    if (totNumRecSteps == 1)
    {
        recurrenceMatrix[0][0]=0.0;
        return;
    }

    scalar normIJ(0.0);
    scalar maxNormIJ(0.0);

    /*
        total number of computed elements: total number of matrix entries
            minus the main diagonal entries, divided by two,
            since the matrix is symmetric
    */
    label size = (totNumRecSteps*(totNumRecSteps-1))/2;
    label counter = 0;
    label percentage = 0;


    label N(this->base_.recM().numRecFields());
    label M(this->base_.recM().numDataBaseFields());

    if (verbose_)
    {
        Info << " N = " << N << ",  M = " << M << endl;
    }

    label ti(0);
    label tj(0);
    label tmp(-1);

    for (int j=0; j<=N/(M-1); j++)
    {
        for (int i=0; i<totNumRecSteps; i++)
        {
            if (verbose_)
            {
                Info << " i = " << i << ",  j = " << j << endl;
            }

            if(counter >= 0.1 * percentage * size)
            {
                Info << "\t" << 10 * percentage << " \% done" << endl;
                percentage++;
            }

            for (int k=i; k<i+(M-1); k++)
            {
                ti = i;
                tj = j*(M-1) + k;

                if (ti > tj)
                {
                    tmp = ti;
                    ti = tj;
                    tj = tmp;
                }

                // skip coordinates outside the recurrence space
                if (ti >= N or tj >= N)
                {
                    continue;
                }

                // start
                // skip main diagonal and upper half
                if (ti >= tj)
                {
                    recurrenceMatrix[ti][tj] = 0;
                    continue;
                }

                if (verbose_)
                {
                    Info << " Doing calculation for element "
                        << ti << " " << tj << endl;
                }

                counter++;

                // compute elements
                if (fieldType_ == "volScalarField")
                {
                    normIJ = normVSF(ti,tj);
                }
                else if (fieldType_ == "volVectorField")
                {
                    normIJ = normVVF(ti,tj);
                }
                else if (fieldType_ == "surfaceScalarField")
                {
                    normIJ = normSSF(ti,tj);
                }
                else
                {
                    FatalError
                        << "diffNorm: Unknown field type " << fieldType_
                        << abort(FatalError);
                }

                recurrenceMatrix[ti][tj] = normIJ;

                if (normIJ > maxNormIJ)
                {
                    maxNormIJ = normIJ;
                }
                // end
            }
        }
    }


    // normalize matrix and copy lower to upper half
    if (normConstant_ > 0.0) maxNormIJ = normConstant_;
    if (maxNormIJ < SMALL)
    {
        Info << "Small normalization constant detected. Setting maxNormIJ = 1.0." << endl;
        maxNormIJ = 1.0;
    }

    for(label ti=0;ti<totNumRecSteps;ti++)
    {
        for(label tj=0;tj<totNumRecSteps;tj++)
        {
            if (ti >= tj) continue;

            if (recurrenceMatrix[ti][tj] < 0)
            {
                FatalErrorInFunction << "Error in computation of recurrence matrix!"
                    << nl << "Negative elements encountered. This should not happen!"
                    << abort(FatalError);
            }

            recurrenceMatrix[ti][tj] /= maxNormIJ;
            recurrenceMatrix[tj][ti] = recurrenceMatrix[ti][tj];
        }
    }

    Info << "\nComputing recurrence matrix done\n" << endl;
}

void diffNorm::restrictToIntegrationDomain(volScalarField &vsf)
{
    if (!existIntegrationDomain_) return;

    forAll(vsf, cellI)
    {
        if (!integrationDomain_()[cellI])
        {
            vsf[cellI] *= 0.0;
        }
    }
}

void diffNorm::restrictToIntegrationDomain(volVectorField &vvf)
{
    if (!existIntegrationDomain_) return;

    forAll(vvf, cellI)
    {
        if (!integrationDomain_()[cellI])
        {
            vvf[cellI] *= 0.0;
        }
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
