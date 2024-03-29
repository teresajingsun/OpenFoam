/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2019-2020 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::LESModels::dynamicCubicRootRS

Group
    grpLESTurbulence

Description
    The dynamicCubicRootRS SGS model.


    Implementation of the dynamic Smagorinsky SGS model 
                     as proposed by Lilly (1992) for OpenFOAM
Copyright Information
    Copyright (C) 1991-2009 OpenCFD Ltd.
    Copyright (C) 2010-2021 Alberto Passalacqua 
License
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
Class
    Foam::LESModels::dynamicCubicRootRS
Group
    grpLESTurbulence
Description
    The isochoric dynamic Smagorinsky model for incompressible flows.
    Algebraic eddy viscosity SGS model founded on the assumption that
    local equilibrium prevails.
    Thus,
    @verbatim
        B = 2/3*k*I - 2*nuSgs*dev(D)
        Beff = 2/3*k*I - 2*nuEff*dev(D)
    where
        k = cI*delta^2*||D||^2
        nuSgs = cD*delta^2*||D||
        nuEff = nuSgs + nu
    In the dynamic version of the choric  Smagorinsky model
    the coefficients cI and cD are calculated during the simulation,
        cI=<K*m>_face/<m*m>_face
    and
        cD=<L.M>_face/<M.M>_face,
    where
        K = 0.5*(F(U.U) - F(U).F(U))
        m = delta^2*(4*||F(D)||^2 - F(||D||^2))
        L = dev(F(U*U) - F(U)*F(U))
        M = delta^2*(F(||D||*dev(D)) - 4*||F(D)||*F(dev(D)))
        <...>_face = face average
    @endverbatim
SourceFiles
    dynamicCubicRootRS.C
    
Authors
    Alberto Passalacqua <apcfd@outlook.com>
References
    Lilly, D. K., A proposed modificaiton of the Germano subgrid-scale 
	closure method, Physics of Fluid A, 4 (3), 1992.
Notes
    Implementation of the dynamic Smagorinsky model with coefficients cD and
    cI computed as local average of their face values to avoid numerical 
    instabilities. 
    Negative values of the effective viscosity are removed by clipping it to
    zero (nuSgs is clipped to -nu)
    
    The code is known to work with OpenFOAM v2012.
\*---------------------------------------------------------------------------*/

#ifndef dynamicCubicRootRS_H
#define dynamicCubicRootRS_H

#include "LESModel.H"
#include "LESfilter.H"
#include "LESeddyViscosity.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace LESModels
{

/*---------------------------------------------------------------------------*\
                    Class dynamicCubicRootRS Declaration
\*---------------------------------------------------------------------------*/

template<class BasicTurbulenceModel>
class dynamicCubicRootRS : public LESeddyViscosity<BasicTurbulenceModel>
{
    // Private Member Functions

        void calcCD(const volSymmTensorField& S);

        void calcCI(const volSymmTensorField& S);

        //- No copy construct
        dynamicCubicRootRS(const dynamicCubicRootRS&) = delete;

        //- No copy assignment
        void operator=(const dynamicCubicRootRS&) = delete;


protected:

    // Protected data

        volScalarField cD_;
        volScalarField cI_;

        autoPtr<LESfilter> filterPtr_;
        LESfilter& filter_;

    // Protected Member Functions

        //- Update sub-grid eddy-viscosity
        void correctNut(const tmp<volTensorField>& gradU);

        virtual void correctNut();


public:

    typedef typename BasicTurbulenceModel::alphaField alphaField;
    typedef typename BasicTurbulenceModel::rhoField rhoField;
    typedef typename BasicTurbulenceModel::transportModel transportModel;

    //- Runtime type information
    TypeName("dynamicCubicRootRS");


    // Constructors

        //- Construct from components
        dynamicCubicRootRS
        (
            const alphaField& alpha,
            const rhoField& rho,
            const volVectorField& U,
            const surfaceScalarField& alphaRhoPhi,
            const surfaceScalarField& phi,
            const transportModel& transport,
            const word& propertiesName = turbulenceModel::propertiesName,
            const word& type = typeName
        );


    //- Destructor
    virtual ~dynamicCubicRootRS() = default;


    // Member Functions

        //- Read model coefficients if they have changed
        virtual bool read();

        //- Return SGS kinetic energy
        tmp<volScalarField> k(const tmp<volTensorField>& gradU) const
        {
            return cI_*sqr(this->delta())*magSqr(dev(symm(gradU)));
        }

        //- Return SGS kinetic energy
        virtual tmp<volScalarField> k() const
        {
            return k(fvc::grad(this->U_));
        }

        //- Correct Eddy-Viscosity and related properties
        virtual void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LESModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "dynamicCubicRootRS.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
