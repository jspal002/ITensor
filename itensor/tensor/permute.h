//
// Distributed under the ITensor Library License, Version 1.1.
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_PERMUTE_H
#define __ITENSOR_PERMUTE_H

#include <array>
#include "itensor/tensor/ten.h"
#include "itensor/tensor/types.h"
#include "itensor/tensor/permutation.h"
#include "itensor/util/safe_ptr.h"
#include "itensor/detail/gcounter.h"

namespace itensor {

template<typename R1, typename R2>
void 
permute(const TenRefc<R1>& from, 
        const Permutation& P, 
        const TenRef<R2>& to);

template<typename R>
void 
permute(const TenRefc<R>& from, 
        const Permutation& P, 
        Ten& to);

template<typename R>
Ten 
permute(const TenRefc<R>& from, 
        const Permutation& P);

//Callable is any function func(Real& x, Real y)
//default is func = [](Real& x, Real y) { x = y; };
template<typename T, typename R1, typename R2, typename Callable>
void 
permute(TensorRef<const T,R1> from, 
        const Permutation& P, 
        TensorRef<T,R2> to,
        const Callable& func);

template<typename R1, typename R2, typename Callable>
void 
permute(const TenRefc<R1>& from, 
        const Label& fL, 
        const TenRef<R2>& to,
        const Label& tL, 
        const Callable& func);

///
/// Implementations
///


template<typename T, typename R1, typename R2, typename Callable>
void 
permute(TensorRef<const T,R1> from, 
        const Permutation& P, 
        TensorRef<T,R2> to,
        const Callable& func)
    {
    using size_type = decltype(P.size());
    auto r = P.size();
#ifdef DEBUG
    if(r != from.r()) throw std::runtime_error("Mismatched Permutation size in permute");
    if(to.r() != from.r()) throw std::runtime_error("Mismatched tensor ranks in permute");
    if(to.size() != from.size()) throw std::runtime_error("Mismatched storage sizes in permute");
    for(size_type j = 0; j < r; ++j)
        {
        if(to.extent(P.dest(j)) != from.extent(j))
            throw std::runtime_error("Incompatible extents in permute");
        }
#endif

    if(r == 0)
        {
        func(*to.data(),*from.data());
        return;
        }

    //find largest index of from,
    //size "bigsize" and position "bigind"
    size_type bigind = 0, 
              bigsize = from.extent(0);
    for(size_type j = 1; j < r; ++j)
        if(bigsize < from.extent(j))
            {
            bigsize = from.extent(j); 
            bigind = j;
            }

    auto stepfrom = from.stride(bigind);
    auto stepto = to.stride(P.dest(bigind));

    detail::GCounter c(0,r-1,0);
    for(size_type i = 0; i < r; ++i)
        c.setInd(i,0,from.extent(i)-1);
    //Leave bigind fixed to zero, will
    //increment manually in the loop below
    c.setInd(bigind,0,0);

    Label ti(r);
    for(; c.notDone(); ++c)
        {
        for(size_type j = 0; j < r; ++j)
            ti[P.dest(j)] = c.i[j];

        auto pto = MAKE_SAFE_PTR3(to.data(),ind(to,ti),to.size());
        auto pfrom = MAKE_SAFE_PTR3(from.data(),ind(from,c.i),from.size());
        for(size_type b = 0; b < bigsize; ++b)
            {
            //func defaults to (*pto = *pfrom) but can also 
            //be operations such as (*pto += *pfrom)
            func(*pto,*pfrom);
            pto += stepto;
            pfrom += stepfrom;
            }
        }
    }

namespace detail {
template<typename T>
void 
assign(T& r1, T r2) { r1 = r2; }
template<typename T>
void
plusEq(T& r1, T r2) { r1 += r2; }
}

template<typename R1, typename R2>
void 
permute(const TenRefc<R1>& from, 
        const Permutation& P, 
        const TenRef<R2>& to)
    {
    permute(from,P,to,detail::assign<Real>);
    }

template<typename R>
void 
permute(const TenRefc<R>& from, 
        const Permutation& P, 
        Ten& to)
    {
    permute(from,P,makeRef(to));
    }

template<typename R>
Ten 
permute(const TenRefc<R>& from, 
        const Permutation& P)
    {
    Range::storage_type rstore(from.r());
    for(size_t j = 0; j < rstore.size(); ++j)
        {
        rstore[P.dest(j)].ext = from.extent(j);
        }
    auto to = Ten(Range(std::move(rstore)));
    permute(from,P,makeRef(to));
    return to;
    }

template<typename R1, typename R2, typename Callable>
void 
permute(const TenRefc<R1>& from, 
        const Label& fL, 
        const TenRef<R2>& to,
        const Label& tL, 
        const Callable& func)
    {
#ifdef DEBUG
    if(fL.size() != tL.size()) throw std::runtime_error("Mismatched sizes in permute");
#endif
    if(fL.empty())
        {
        *to.data() = *from.data();
        return;
        }
    Permutation P(fL.size());
    calc_permutation(fL,tL,P);
    permute(from,P,to,func);
    }

template<typename R1, typename R2>
void 
permute(const TenRefc<R1>& from, 
        const Label& fL, 
        const TenRef<R2>& to,
        const Label& tL)
    {
    permute(from,fL,to,tL,detail::assign<Real>);
    }

} //namespace itensor

#endif
